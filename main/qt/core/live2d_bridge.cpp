#include "spinelove/live2d_bridge.h"
#include "spinelove/interaction_rules.h"
#include "petting_tracker.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdio>
#include <QFileInfo>
#include <QJSValue>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <vector>
#include <unordered_map>

#if defined(Q_OS_WIN)
#include "../../live2d/live2d_module.h"
#include "../../render_d3d11/d3d11_renderer.h"
#include "../../render_d3d11/d3d11_texture.h"
#include <objbase.h>
#include <wrl/client.h>
#endif

namespace {
struct Command { QString name; QVariant value; };
constexpr std::array<const char*, 6> GazeKeys{"angleX", "angleY", "angleZ", "bodyX", "eyeX", "eyeY"};
QVariant CommandData(const QVariant& value) {
    if (value.metaType() == QMetaType::fromType<QJSValue>())
        return CommandData(value.value<QJSValue>().toVariant(QJSValue::ConvertJSObjects));
    switch (value.typeId()) {
    case QMetaType::QVariantMap: {
        auto result = value.toMap();
        for (auto it = result.begin(); it != result.end(); ++it) it.value() = CommandData(it.value());
        return result;
    }
    case QMetaType::QVariantHash: {
        auto result = value.toHash();
        for (auto it = result.begin(); it != result.end(); ++it) it.value() = CommandData(it.value());
        return result;
    }
    case QMetaType::QVariantList: {
        auto result = value.toList();
        for (auto& item : result) item = CommandData(item);
        return result;
    }
    default:
        return value;
    }
}
float Number(const QVariant& value, float fallback = 0.0f) {
    bool ok = false;
    const float result = value.toFloat(&ok);
    return ok && std::isfinite(result) ? result : fallback;
}
int Index(const QVariant& value, std::size_t count) {
    bool ok = false;
    const int index = value.toInt(&ok);
    return ok && index >= 0 && static_cast<std::size_t>(index) < count ? index : -1;
}
}

struct Live2DBridge::Impl {
    mutable QMutex mutex;
    std::deque<Command> pending;
    QVariantMap snapshot{{"mode", "live2d"}, {"loaded", false}, {"backendAvailable", false}};
    Qt::HANDLE renderThread = nullptr;
    QSize size;
    QString error;
    QString lastManifest;
    QElapsedTimer publishClock;
    quint64 revision = 0;
    quint64 publishedRevision = 0;
    QVariantList nativeHitResults;
    QVariant exportAckId;
    bool exportRequestSuccess = false;
    QString exportError;
    QString acknowledgedExportCommand;
    quint64 exportFrameSerial = 0;
#if defined(Q_OS_WIN)
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    std::unique_ptr<sl_d3d11::D3D11Renderer> textures;
    live2d::Live2DModule module;
    SlTextureId target = 0;
    bool comOwned = false;
    std::vector<int> queue;
    bool queuePlaying = false;
    std::size_t queueIndex = 0;
    bool forceExportFrame = false;
    quint64 deviceGeneration = 0, recoveryCount = 0;
    struct RecoveryState {
        bool valid = false;
        QString manifest;
        std::string motion, expression;
        float scale = 1, speed = 1, offsetX = 0, offsetY = 0, volume = 0.8f;
        bool loopAll = false, queuePlaying = false;
        std::size_t queueIndex = 0;
        live2d::EffectSettings effects;
        live2d::DragSettings gaze;
        live2d::GazePose gazePose;
        std::vector<live2d::ParameterState> parameters;
        std::vector<live2d::PartState> parts;
        std::vector<std::string> queue;
    } recovery;
    slqt::detail::PettingTracker petting;
    QVariantList animationCatalog, expressionCatalog;
    unsigned catalogGeneration = ~0u;

    void stopQueue() {
        const bool wasPlaying = queuePlaying;
        queuePlaying = false;
        queueIndex = 0;
        if (wasPlaying && module.HasImportedModel() && module.CurrentMotionIndex() >= 0)
            module.PlayMotion(static_cast<std::size_t>(module.CurrentMotionIndex()));
    }

    bool open(const QString& path) {
        nativeHitResults.clear();
        recovery = {};
        stopQueue();
        petting.reset();
        queue.clear();
        queueIndex = 0;
        const bool ok = module.ImportModel(path.toStdWString());
        lastManifest = ok ? path : QString{};
        error = ok ? QString{} : QString::fromUtf8(module.LastError().c_str());
        return ok;
    }

    void rememberForDeviceRecovery() {
        if (!module.HasImportedModel()) return;
        if (module.ExportSessionActive()) {
            module.EndExportSession();
            exportRequestSuccess = false;
            exportError = "Live2D rendering was interrupted. Restart the export after the device recovers.";
        }
        RecoveryState next;
        next.valid = true; next.manifest = QString::fromStdWString(module.ManifestPath());
        next.scale = module.ModelScale(); next.speed = module.TimeScale();
        next.offsetX = module.ViewOffsetX(); next.offsetY = module.ViewOffsetY();
        next.volume = module.VoiceVolume(); next.loopAll = module.LoopAll();
        next.effects = module.Effects(); next.gaze = module.GetDragSettings();
        next.gazePose = module.GetGazePose();
        next.parameters = module.Parameters(); next.parts = module.Parts();
        const int motion = module.CurrentMotionIndex(), expression = module.CurrentExpressionIndex();
        if (motion >= 0 && static_cast<std::size_t>(motion) < module.MotionNames().size()) next.motion = module.MotionNames()[motion];
        if (expression >= 0 && static_cast<std::size_t>(expression) < module.ExpressionNames().size()) next.expression = module.ExpressionNames()[expression];
        next.queuePlaying = queuePlaying; next.queueIndex = queueIndex;
        for (int index : queue) if (index >= 0 && static_cast<std::size_t>(index) < module.MotionNames().size()) next.queue.push_back(module.MotionNames()[index]);
        recovery = std::move(next);
    }

    void restoreAfterDeviceRecovery() {
        if (!recovery.valid) return;
        RecoveryState saved = std::move(recovery);
        if (!open(saved.manifest)) { recovery = std::move(saved); return; }
        module.SetModelScale(saved.scale); module.SetTimeScale(saved.speed);
        module.SetViewOffset(saved.offsetX, saved.offsetY); module.SetVoiceVolume(saved.volume);
        module.SetLoopAll(saved.loopAll); module.SetEffects(saved.effects); module.SetDragSettings(saved.gaze);
        module.SetGazePose(saved.gazePose);
        const auto& motions = module.MotionNames();
        const auto motionIndex = [&motions](const std::string& name) {
            const auto found = std::find(motions.begin(), motions.end(), name);
            return found == motions.end() ? -1 : static_cast<int>(found - motions.begin());
        };
        const int selectedMotion = motionIndex(saved.motion);
        if (selectedMotion >= 0) module.PlayMotion(selectedMotion);
        const auto& expressions = module.ExpressionNames();
        const auto expression = std::find(expressions.begin(), expressions.end(), saved.expression);
        if (expression != expressions.end()) module.PlayExpression(static_cast<std::size_t>(expression - expressions.begin()));
        std::unordered_map<std::string, float> parameters, parts;
        for (const auto& p : saved.parameters) if (p.overridden) parameters.emplace(p.id, p.value);
        for (const auto& p : saved.parts) if (p.overridden) parts.emplace(p.id, p.opacity);
        for (std::size_t i = 0; i < module.Parameters().size(); ++i) {
            const auto found = parameters.find(module.Parameters()[i].id);
            if (found != parameters.end()) module.SetParameter(i, found->second);
        }
        for (std::size_t i = 0; i < module.Parts().size(); ++i) {
            const auto found = parts.find(module.Parts()[i].id);
            if (found != parts.end()) module.SetPartOpacity(i, found->second);
        }
        queue.clear();
        for (const auto& name : saved.queue) { const int index = motionIndex(name); if (index >= 0) queue.push_back(index); }
        queueIndex = queue.empty() ? 0 : (std::min)(saved.queueIndex, queue.size() - 1);
        queuePlaying = saved.queuePlaying && !queue.empty() && module.PlayMotionOnce(queue[queueIndex]);
        ++recoveryCount;
    }

    void setGazeMode(bool follow) {
        if (!module.HasImportedModel()) return;
        auto pose = module.GetGazePose();
        if (!follow && !pose.enabled) {
            const auto indices = module.GazeParameterIndices();
            const auto& parameters = module.Parameters();
            for (std::size_t i = 0; i < indices.size(); ++i)
                pose.values[i] = indices[i] >= 0 ? parameters[indices[i]].value : 0.0f;
        }
        pose.enabled = !follow;
        module.SetGazePose(pose);
        auto effects = module.Effects(); effects.gazeFollow = follow; module.SetEffects(effects);
        module.EndDrag();
    }

    void apply(const Command& c) {
        const auto& name = c.name;
        const auto& value = c.value;
        const QVariantMap map = value.toMap();
        const QString key = map.value("key").toString();
        if (name == "live2d.open" || name == "file.play") { open(value.toString()); return; }
        if (name == "live2d.clear") { stopQueue(); queue.clear(); petting.reset(); recovery = {}; nativeHitResults.clear(); module.Clear(); lastManifest.clear(); return; }
        if (name == "live2d.suspend") { stopQueue(); module.EndDrag(); petting.reset(); return; }
        if (name == "live2d.pausePreview") { module.StopVoice(); module.EndDrag(); petting.reset(); return; }
        if (name == "view.scale") module.SetModelScale(Number(value, 1));
        else if (name == "view.zoom") {
            bool validSteps = false;
            const int steps = map.value("steps").toInt(&validSteps);
            if (!validSteps || !steps) return;
            const float oldScale = module.ModelScale();
            const float scale = slqt::interaction::wheelScale(oldScale, steps, map.value("inverted").toBool());
            module.SetModelScale(scale);
            if (!map.value("centerOnly").toBool() && oldScale > 0 && scale != oldScale) {
                const float originX = Number(map.value("originX"));
                const float originY = Number(map.value("originY"));
                const float anchorX = originX + module.ViewOffsetX();
                const float anchorY = originY + module.ViewOffsetY();
                const float ratio = scale / oldScale;
                module.SetViewOffset(slqt::interaction::anchorAfterScale(anchorX, Number(map.value("x")), ratio) - originX,
                    slqt::interaction::anchorAfterScale(anchorY, Number(map.value("y")), ratio) - originY);
            }
        }
        else if (name == "playback.speed") module.SetTimeScale(Number(value, 1));
        else if (name == "view.reset") module.ResetView();
        else if (name == "view.pan") module.PanByPixels(Number(map.value("x")), Number(map.value("y")));
        else if (name == "view.offset") module.SetViewOffset(Number(map.value("x")), Number(map.value("y")));
        else if (name == "live2d.drag") module.SetDragTarget(Number(map.value("x")), Number(map.value("y")));
        else if (name == "live2d.endDrag") module.EndDrag();
        else if (name == "pet.reset") petting.reset();
        else if (name == "pet.pointer") {
            bool validTime = false;
            const auto nowMs = map.value("nowMs").toULongLong(&validTime);
            if (!validTime || !map.contains("screenX") || !map.contains("screenY")) { petting.reset(); return; }
            const float x = Number(map.value("x"), 2), y = Number(map.value("y"), 2);
            const bool inside = x >= -1 && x <= 1 && y >= -1 && y <= 1;
            const QString area = inside ? QString::fromUtf8(module.HitAreaAt(x, y).c_str()) : QString{};
            if (petting.update(area.startsWith("head", Qt::CaseInsensitive), map.value("dragging").toBool(),
                Number(map.value("screenX")), Number(map.value("screenY")), nowMs)) module.PlayRandomExpression();
        }
        else if(name=="native.raycast"){
            std::vector<live2d::NativeHitCandidate> candidates;
            for(const auto& value:map.value("candidates").toList()){const auto row=value.toMap();live2d::NativeHitCandidate item;item.drawableId=row.value("drawableId").toString().toStdString();item.index=row.value("index",row.value("drawableIndex",-1)).toInt();item.precision=row.value("precision").toInt();item.partType=row.value("partType").toInt();item.enabled=row.value("enabled",true).toBool();candidates.push_back(std::move(item));}
            QVariantList hits;for(const auto& hit:module.NativeRaycast(Number(map.value("x")),Number(map.value("y")),candidates))hits.append(QVariantMap{{"drawableId",QString::fromStdString(hit.drawableId)},{"index",hit.index},{"partType",hit.partType}});
            nativeHitResults.append(QVariantMap{{"token",map.value("token")},{"epoch",map.value("epoch")},{"hits",hits}});while(nativeHitResults.size()>64)nativeHitResults.removeFirst();
        }
        else if (name == "live2d.tap") { stopQueue(); module.TapAt(Number(map.value("x")), Number(map.value("y"))); }
        else if (name == "animation.play") {
            const int index = Index(value, module.MotionNames().size());
            if (index >= 0 && !module.ExportSessionActive()) { stopQueue(); module.PlayMotion(index); }
        }
        else if (name == "animation.native") {
            const auto motion = map.value("name").toString().toUtf8().toStdString();
            const auto& names = module.MotionNames();
            const auto found = std::find_if(names.begin(), names.end(), [&motion](const std::string& name){
                return name == motion || name.rfind(motion + " / ", 0) == 0;
            });
            if (found != names.end()) {
                stopQueue(); const auto index = std::size_t(found - names.begin());
                module.SetLoopAll(map.value("loop", true).toBool());
                module.PlayNativeMotion(index,map.value("loop",true).toBool(),map.value("mix").toDouble(),map.value("time",-1).toDouble());
            }
        }
        else if(name=="animation.nativeTime")module.SeekNativeMotion(value.toDouble());
        else if(name=="animation.nativeLayers"){
            std::vector<std::string> names;for(const auto& v:value.toList())names.push_back(v.toString().toUtf8().toStdString());stopQueue();module.SetNativeLayers(names);
        }
        else if(name=="animation.nativeLayerStates"){
            std::vector<live2d::NativeLayerState> layers;std::unordered_map<std::string,float> overrides,parts;
            for(const auto& value:map.value("layers").toList()){
                const auto row=value.toMap();live2d::NativeLayerState layer;
                layer.name=row.value("name").toString().toStdString();layer.previousName=row.value("previousName").toString().toStdString();
                layer.time=row.value("time").toDouble();layer.previousTime=row.value("previousTime").toDouble();
                layer.weight=Number(row.value("weight"),1);layer.mixWeight=Number(row.value("mixWeight"),1);
                layer.loop=row.value("loop",true).toBool();layer.previousLoop=row.value("previousLoop",true).toBool();
                layer.writeDefaults=row.value("writeDefaults",true).toBool();layer.previousWriteDefaults=row.value("previousWriteDefaults",true).toBool();
                layers.push_back(std::move(layer));
            }
            const auto values=map.value("overrides").toMap();for(auto it=values.cbegin();it!=values.cend();++it)overrides[it.key().toStdString()]=Number(it.value());
            const auto partValues=map.value("parts").toMap();for(auto it=partValues.cbegin();it!=partValues.cend();++it)parts[it.key().toStdString()]=Number(it.value());
            stopQueue();module.SetNativeLayerStates(layers,overrides,parts);
        }
        else if(name=="view.nativeMatrix"){
            const auto v=value.toList();if(v.size()==6){std::array<float,6> matrix;for(int i=0;i<6;++i)matrix[i]=Number(v[i]);module.SetSceneTransform(matrix);}
        }
        else if (name == "animation.step") {
            bool valid = false;
            const int steps = value.toInt(&valid);
            const auto count = static_cast<qint64>(module.MotionNames().size());
            if (valid && steps != 0 && count > 0) {
                const qint64 current = module.CurrentMotionIndex() >= 0 ? module.CurrentMotionIndex() : (steps > 0 ? -1 : 0);
                const auto index = static_cast<std::size_t>(((current + steps) % count + count) % count);
                stopQueue(); module.PlayMotion(index);
            }
        }
        else if (name == "live2d.volume") module.SetVoiceVolume(Number(value, 0.8f));
        else if (name == "live2d.loopAll") module.SetLoopAll(value.toBool());
        else if (name == "live2d.expression") {
            const int index = Index(value, module.ExpressionNames().size());
            if (index >= 0) module.PlayExpression(index);
        }
        else if (name == "live2d.randomExpression") module.PlayRandomExpression();
        else if (name == "live2d.clearExpression") module.ClearExpression();
        else if (name == "live2d.clearOverrides") module.ClearParameterOverrides();
        else if (name == "live2d.resetGaze") module.ResetDragSettings();
        else if (name == "live2d.gazeMode") setGazeMode(value.toBool());
        else if (name == "live2d.gazePose" || name == "live2d.resetGazePose") {
            auto pose = module.GetGazePose();
            if (!pose.enabled) return;
            const auto indices = module.GazeParameterIndices();
            const auto& parameters = module.Parameters();
            for (std::size_t i = 0; i < GazeKeys.size(); ++i) {
                if (indices[i] < 0 || parameters[indices[i]].overridden) continue;
                if (name == "live2d.resetGazePose") pose.values[i] = parameters[indices[i]].defaultValue;
                else if (key == QLatin1String(GazeKeys[i])) pose.values[i] = Number(map.value("value"), pose.values[i]);
            }
            module.SetGazePose(pose);
        }
        else if (name == "live2d.effect") {
            auto effects = module.Effects();
            const bool enabled = map.value("value").toBool();
            if (key == "eyeBlink") effects.eyeBlink = enabled;
            else if (key == "breath") effects.breath = enabled;
            else if (key == "physics") effects.physics = enabled;
            else if (key == "lipSync") effects.lipSync = enabled;
            else if (key == "gazeFollow") {
                effects.gazeFollow = enabled;
                auto pose = module.GetGazePose(); pose.enabled = false; module.SetGazePose(pose);
            }
            module.SetEffects(effects);
        }
        else if (name == "live2d.gaze") {
            auto gaze = module.GetDragSettings();
            const float v = Number(map.value("value"));
            if (key == "sensitivity") gaze.sensitivity = v;
            else if (key == "angleX") gaze.angleX = v;
            else if (key == "angleY") gaze.angleY = v;
            else if (key == "angleZ") gaze.angleZ = v;
            else if (key == "bodyX") gaze.bodyAngleX = v;
            else if (key == "eyeX") gaze.eyeBallX = v;
            else if (key == "eyeY") gaze.eyeBallY = v;
            module.SetDragSettings(gaze);
        }
        else if (name == "live2d.parameterValue" || name == "live2d.parameterOverride") {
            const int index = Index(map.value("index"), module.Parameters().size());
            if (index >= 0) {
                if (name == "live2d.parameterValue") module.SetParameter(index, Number(map.value("value")));
                else module.SetParameterOverride(index, map.value("value").toBool());
            }
        }
        else if (name == "live2d.parameterReset") {
            const int index = Index(value, module.Parameters().size());
            if (index >= 0) module.ResetParameter(index);
        }
        else if (name == "live2d.partValue" || name == "live2d.partOverride") {
            const int index = Index(map.value("index"), module.Parts().size());
            if (index >= 0) {
                if (name == "live2d.partValue") module.SetPartOpacity(index, Number(map.value("value")));
                else module.SetPartOverride(index, map.value("value").toBool());
            }
        }
        else if (name == "live2d.partReset") {
            const int index = Index(value, module.Parts().size());
            if (index >= 0) module.ResetPart(index);
        }
        else if (name == "queue.stop") { if (!module.ExportSessionActive()) stopQueue(); }
        else if (name == "queue.clear") {
            if (!module.ExportSessionActive()) { stopQueue(); queue.clear(); }
        }
        else if (name == "queue.add") {
            const int index = Index(value, module.MotionNames().size());
            if (index >= 0 && !queuePlaying && !module.ExportSessionActive()) queue.push_back(index);
        }
        else if (name == "queue.remove") {
            const int index = Index(value, queue.size());
            if (index >= 0 && !queuePlaying && !module.ExportSessionActive()) queue.erase(queue.begin() + index);
        }
        else if (name == "queue.play" && !queue.empty() && !module.ExportSessionActive()) {
            queueIndex = 0;
            queuePlaying = module.PlayMotionOnce(queue.front());
        }
    }

    void advanceQueue() {
        if (!queuePlaying || module.ExportSessionActive() || queue.empty() || !module.IsMotionFinished()) return;
        for (std::size_t i = 0; i < queue.size(); ++i) {
            queueIndex = (queueIndex + 1) % queue.size();
            if (module.PlayMotionOnce(queue[queueIndex])) return;
        }
        stopQueue();
    }

    void publish(bool force = false) {
        if (!force && publishClock.isValid() && publishClock.elapsed() < 33) return;
        publishClock.restart();
        const bool loaded = module.HasImportedModel();
        if (catalogGeneration != module.ModelGeneration()) {
            catalogGeneration = module.ModelGeneration();
            animationCatalog.clear(); expressionCatalog.clear();
            const auto& names = module.MotionNames();
            for (std::size_t i = 0; i < names.size(); ++i)
                animationCatalog.push_back(QVariantMap{{"name", QString::fromUtf8(names[i].c_str())}, {"duration", module.MotionDuration(i)}});
            for (const auto& expression : module.ExpressionNames()) expressionCatalog.push_back(QString::fromUtf8(expression.c_str()));
        }
        QVariantList parameters, parts, queueRows;
        for (const auto& p : module.Parameters())
            parameters.push_back(QVariantMap{{"id", QString::fromUtf8(p.id.c_str())}, {"name", QString::fromUtf8(p.displayName.c_str())},
                {"value", p.value}, {"min", p.minimum}, {"max", p.maximum}, {"defaultValue", p.defaultValue}, {"overridden", p.overridden}});
        for (const auto& p : module.Parts())
            parts.push_back(QVariantMap{{"id", QString::fromUtf8(p.id.c_str())}, {"name", QString::fromUtf8(p.displayName.c_str())},
                {"value", p.opacity}, {"min", 0.0f}, {"max", 1.0f}, {"defaultValue", p.defaultOpacity}, {"overridden", p.overridden}});
        for (int index : queue)
            if (index >= 0 && index < animationCatalog.size()) queueRows.push_back(animationCatalog[index]);
        const auto effects = module.Effects();
        const auto gaze = module.GetDragSettings();
        const auto pose = module.GetGazePose();
        const auto gazeIndices = module.GazeParameterIndices();
        const auto& modelParameters = module.Parameters();
        QVariantMap gazeChannels;
        for (std::size_t i = 0; i < GazeKeys.size(); ++i) {
            QVariantMap channel{{"supported", gazeIndices[i] >= 0}};
            if (gazeIndices[i] >= 0) {
                const auto& parameter = modelParameters[gazeIndices[i]];
                channel.insert("value", pose.enabled ? pose.values[i] : parameter.value);
                channel.insert("min", parameter.minimum); channel.insert("max", parameter.maximum);
                channel.insert("locked", parameter.overridden);
            }
            gazeChannels.insert(QString::fromLatin1(GazeKeys[i]), channel);
        }
        const bool editable = loaded && !module.ExportSessionActive();
        QVariantMap capabilities;
        for (const char* command : {"view.scale", "playback.speed", "view.reset", "animation.play", "live2d.volume", "live2d.loopAll",
            "live2d.clearExpression", "live2d.clearOverrides", "live2d.resetGaze", "live2d.effect", "live2d.gaze", "live2d.gazeMode",
            "live2d.partValue", "live2d.partReset", "live2d.parameterValue", "live2d.parameterOverride", "live2d.parameterReset"})
            capabilities.insert(QString::fromLatin1(command), editable);
        capabilities.insert("live2d.gazePose", editable && pose.enabled);
        capabilities.insert("live2d.resetGazePose", editable && pose.enabled);
        capabilities.insert("live2d.expression", editable && !expressionCatalog.empty());
        capabilities.insert("queue.add", loaded && !queuePlaying && !module.ExportSessionActive());
        capabilities.insert("queue.remove", loaded && !queuePlaying && !module.ExportSessionActive());
        capabilities.insert("queue.play", loaded && !queue.empty() && !module.ExportSessionActive());
        capabilities.insert("queue.stop", queuePlaying && !module.ExportSessionActive());
        capabilities.insert("queue.clear", !queue.empty() && !module.ExportSessionActive());
        QVariantMap result{{"mode", "live2d"}, {"loaded", loaded}, {"backendAvailable", module.RenderingBackendAvailable()},
            {"nativeHitResults",nativeHitResults},
            {"exportAckId", exportAckId}, {"exportRequestId", exportAckId}, {"exportRequestSuccess", exportRequestSuccess},
            {"exportError", exportError}, {"exportFrameSerial", QVariant::fromValue(exportFrameSerial)},
            {"live2dRevision", QVariant::fromValue(++revision)}, {"live2dDeviceGeneration", QVariant::fromValue(deviceGeneration)},
            {"live2dRecoveryCount", QVariant::fromValue(recoveryCount)}, {"currentFileName", QString::fromUtf8(module.DisplayName().c_str())},
            {"currentModelPath", QString::fromStdWString(module.ManifestPath())}, {"error", error},
            {"scale", module.ModelScale()}, {"timeScale", module.TimeScale()}, {"voiceVolume", module.VoiceVolume()},
            {"loopAll", module.LoopAll()}, {"offsetX", module.ViewOffsetX()}, {"offsetY", module.ViewOffsetY()},
            {"canvasWidth", size.width()}, {"canvasHeight", size.height()}, {"animations", animationCatalog},
            {"currentAnimation", module.CurrentMotionIndex()}, {"expressions", expressionCatalog}, {"currentExpression", module.CurrentExpressionIndex()},
            {"parts", parts}, {"parameters", parameters}, {"partCount", parts.size()}, {"parameterCount", parameters.size()},
            {"queue", queueRows}, {"queuePlaying", queuePlaying},
            {"queueIndex", queuePlaying ? static_cast<int>(queueIndex) : 0}, {"queueExporting", module.ExportSessionActive()},
            {"effects", QVariantMap{{"eyeBlink", effects.eyeBlink}, {"breath", effects.breath}, {"physics", effects.physics},
                {"lipSync", effects.lipSync}, {"gazeFollow", effects.gazeFollow}}},
            {"gaze", QVariantMap{{"sensitivity", gaze.sensitivity}, {"angleX", gaze.angleX}, {"angleY", gaze.angleY}, {"angleZ", gaze.angleZ},
                {"bodyX", gaze.bodyAngleX}, {"eyeX", gaze.eyeBallX}, {"eyeY", gaze.eyeBallY}}},
            {"gazeChannels", gazeChannels}, {"capabilities", capabilities}};
        live2d::RenderBounds bounds;
        if (module.QueryLastRenderedBounds(bounds))
            result.insert("renderBounds", QVariantMap{{"x", bounds.x}, {"y", bounds.y}, {"width", bounds.width}, {"height", bounds.height}});
        QMutexLocker lock(&mutex);
        snapshot = std::move(result);
        publishedRevision = revision;
    }
#endif

    bool onRenderThread() const { return renderThread == QThread::currentThreadId(); }
#if defined(Q_OS_WIN)
    bool drawTarget(float advance, bool exporting, int width, int height, float centerOffsetX, float centerOffsetY) {
        if (size != QSize(width, height) || !target) {
            if (target) textures->ReleaseTexture(target);
            target = textures->CreateRenderTarget(width, height);
            size = target ? QSize(width, height) : QSize{};
        }
        if (!target) { error = "Unable to allocate the Live2D render target."; return false; }
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> previousTarget;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> previousDepth;
        context->OMGetRenderTargets(1, &previousTarget, &previousDepth);
        D3D11_VIEWPORT previousViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
        UINT count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        context->RSGetViewports(&count, previousViewports);
        float remaining = advance;
        bool ok = true;
        do {
            const float step = exporting ? (std::min)(0.1f, remaining) : remaining;
            const bool bound = textures->BeginRenderTarget(target, SlVec4(0, 0, 0, 0));
            ok = bound && module.TickAndRender(step, width, height, centerOffsetX, centerOffsetY, true);
            remaining = exporting ? remaining - step : 0;
        } while (ok && remaining > 0.000001f);
        textures->EndFrame();
        ID3D11RenderTargetView* previous = previousTarget.Get();
        context->OMSetRenderTargets(1, &previous, previousDepth.Get());
        if (count) context->RSSetViewports(count, previousViewports);
        return ok;
    }
#endif
};

Live2DBridge::Live2DBridge() : m(std::make_unique<Impl>()) {}
Live2DBridge::~Live2DBridge() { shutdown(); }

void Live2DBridge::command(const QString& name, const QVariant& value) {
    QVariant data = CommandData(value);
    QMutexLocker lock(&m->mutex);
    const bool continuous = name == "live2d.gaze" || name == "live2d.gazePose" || name == "live2d.drag"
        || name == "live2d.parameterValue" || name == "live2d.partValue" || name == "animation.nativeLayerStates";
    if (!m->pending.empty() && continuous) {
        Command& last = m->pending.back();
        const bool sameGazeKey = (name != "live2d.gaze" && name != "live2d.gazePose")
            || last.value.toMap().value("key") == data.toMap().value("key");
        const bool sameValueIndex = (name != "live2d.parameterValue" && name != "live2d.partValue")
            || last.value.toMap().value("index") == data.toMap().value("index");
        if (last.name == name && sameGazeKey && sameValueIndex) {
            last.value = std::move(data);
            return;
        }
    }
    m->pending.push_back({name, std::move(data)});
}
QVariantMap Live2DBridge::state() const { QMutexLocker lock(&m->mutex); return m->snapshot; }
quint64 Live2DBridge::revision() const { QMutexLocker lock(&m->mutex); return m->publishedRevision; }

void Live2DBridge::prefetchTextures(const QString& manifestPath) {
#if defined(Q_OS_WIN)
    QFile file(manifestPath);
    if (!file.open(QIODevice::ReadOnly)) return;
    const auto textures = QJsonDocument::fromJson(file.readAll()).object().value("FileReferences").toObject().value("Textures").toArray();
    const QDir directory = QFileInfo(manifestPath).absoluteDir();
    for (const auto& texture : textures)
        if (!texture.toString().isEmpty()) sl_d3d11::PrefetchTexturePixels(QDir::toNativeSeparators(directory.filePath(texture.toString())).toStdWString().c_str());
#else
    Q_UNUSED(manifestPath);
#endif
}

bool Live2DBridge::initialize(ID3D11Device* device, ID3D11DeviceContext* context, const QString& shaderPath) {
#if defined(Q_OS_WIN)
    if (m->renderThread && !m->onRenderThread()) return false;
    if (m->device.Get() == device && m->module.RenderingBackendAvailable()) return true;
    shutdown();
    m->renderThread = QThread::currentThreadId();
    if (!device || !context) { m->error = "Qt did not provide a D3D11 device/context."; m->publish(true); return false; }
    const HRESULT apartment = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    m->comOwned = SUCCEEDED(apartment);
    if (FAILED(apartment) && apartment != RPC_E_CHANGED_MODE) {
        m->error = "The render thread could not initialize COM for Live2D textures/audio."; m->publish(true); return false;
    }
    m->device = device; m->context = context;
    m->textures = std::make_unique<sl_d3d11::D3D11Renderer>();
    if (!m->textures->Initialize(device, context, shaderPath.toStdWString().c_str())) {
        m->error = "Live2D texture renderer initialization failed (check sprite.hlsl)."; m->publish(true); return false;
    }
    if (!m->module.Initialize(device, context, m->textures.get())) {
        m->error = QString::fromUtf8(m->module.LastError().c_str()); m->publish(true); return false;
    }
    m->error.clear();
    ++m->deviceGeneration;
    if (m->recovery.valid) m->restoreAfterDeviceRecovery();
    else if (!m->lastManifest.isEmpty()) m->open(m->lastManifest);
    m->publish(true);
    return true;
#else
    Q_UNUSED(device); Q_UNUSED(context); Q_UNUSED(shaderPath);
    QMutexLocker lock(&m->mutex);
    m->snapshot.insert("error", "The Live2D GLES/Metal backend has not been integrated on this platform yet.");
    m->snapshot.insert("capabilities", QVariantMap{});
    return false;
#endif
}

bool Live2DBridge::processPendingCommands() {
#if defined(Q_OS_WIN)
    if (!m->onRenderThread() || !m->module.RenderingBackendAvailable() || m->module.ExportSessionActive()) return false;
    std::deque<Command> commands;
    {
        QMutexLocker lock(&m->mutex);
        while (!m->pending.empty() && !m->pending.front().name.startsWith("export.")) {
            commands.push_back(std::move(m->pending.front())); m->pending.pop_front();
        }
    }
    for (const auto& command : commands) m->apply(command);
    if (!commands.empty()) m->publish(true);
    return !commands.empty();
#else
    return false;
#endif
}

bool Live2DBridge::render(float deltaSeconds, int width, int height, float centerOffsetX, float centerOffsetY) {
#if defined(Q_OS_WIN)
    if (!m->onRenderThread() || !m->module.RenderingBackendAvailable()) return false;
    std::deque<Command> commands;
    const bool sessionAtEntry = m->module.ExportSessionActive();
    {
        QMutexLocker lock(&m->mutex);
        while (!m->pending.empty()) {
            auto next = m->pending.begin();
            if (sessionAtEntry) {
                next = std::find_if(m->pending.begin(), m->pending.end(), [](const Command& c) { return c.name.startsWith("export."); });
                if (next == m->pending.end()) break;
            }
            const bool transaction = next->name.startsWith("export.");
            commands.push_back(std::move(*next));
            m->pending.erase(next);
            if (transaction) break;
        }
    }
    bool transaction = false, transactionSuccess = false, transactionNeedsFrame = false;
    QString transactionError;
    QVariant transactionId;
    QString transactionName;
    float fixedDelta = 0.0f;
    for (const auto& command : commands) {
        if (!command.name.startsWith("export.")) { m->apply(command); continue; }
        const auto data = command.value.toMap();
        const auto id = data.value("requestId");
        if (id.isValid() && id == m->exportAckId && command.name == m->acknowledgedExportCommand) continue;
        transaction = true; transactionId = id; transactionName = command.name;
        if (!id.isValid() || id.isNull()) { transactionError = "Export requestId is required."; continue; }
        const auto motionValue = data.contains("motionIndex") ? data.value("motionIndex") : data.value("motion");
        const int index = Index(motionValue, m->module.MotionNames().size());
        if (command.name == "export.sync") {
            transactionSuccess = true;
        } else if (command.name == "export.begin") {
            const float fps = Number(data.value("fps"), -1);
            transactionSuccess = index >= 0 && fps >= 1 && fps <= 120 && beginExportSession(index, fps);
            transactionNeedsFrame = transactionSuccess;
            if (!transactionSuccess) transactionError = "Cannot begin Live2D export: check loaded model, motion, FPS and active session.";
        } else if (command.name == "export.switch") {
            transactionSuccess = index >= 0 && exportSessionSwitchMotion(index);
            transactionNeedsFrame = transactionSuccess;
            if (!transactionSuccess) transactionError = "Cannot switch the Live2D export motion.";
        } else if (command.name == "export.step") {
            const float step = Number(data.value("delta"), -1);
            transactionSuccess = m->module.ExportSessionActive() && step >= 0 && step <= 1;
            transactionNeedsFrame = transactionSuccess;
            if (transactionSuccess) fixedDelta = step;
            else transactionError = "Export step needs an active session and a finite delta between 0 and 1 second.";
        } else if (command.name == "export.end") {
            endExportSession();
            transactionSuccess = true;
        } else transactionError = "Unknown Live2D export transaction.";
    }
    const auto acknowledge = [&](bool rendered) {
        if (!transaction) return;
        m->exportAckId = transactionId;
        m->acknowledgedExportCommand = transactionName;
        m->exportRequestSuccess = transactionSuccess && (!transactionNeedsFrame || rendered);
        m->exportError = !m->exportRequestSuccess && transactionError.isEmpty() ? "Live2D export frame could not be rendered." : transactionError;
        if (m->exportRequestSuccess && transactionNeedsFrame) ++m->exportFrameSerial;
    };
    if (!m->module.HasImportedModel() || width <= 0 || height <= 0) {
        acknowledge(false); m->publish(!commands.empty()); return false;
    }
    if (m->module.ExportSessionActive() && !transactionNeedsFrame && !m->forceExportFrame && m->target) {
        acknowledge(true); m->publish(!commands.empty()); return true;
    }
    const bool exporting = m->module.ExportSessionActive();
    const float advance = exporting || sessionAtEntry || transaction ? fixedDelta
        : (std::isfinite(deltaSeconds) ? (std::max)(0.0f, deltaSeconds) : 0.0f);
    const bool ok = m->drawTarget(advance, exporting, width, height, centerOffsetX, centerOffsetY);
    if (!m->target) { acknowledge(false); m->publish(true); return false; }
    m->advanceQueue();
    if (ok && exporting) m->forceExportFrame = false;
    acknowledge(ok);
    const bool immediateUiPublish = std::any_of(commands.cbegin(), commands.cend(), [](const Command& command) {
        return command.name != "live2d.gaze" && command.name != "live2d.gazePose" && command.name != "live2d.drag"
            && command.name != "live2d.parameterValue" && command.name != "live2d.partValue";
    });
    m->publish(immediateUiPublish);
    return ok;
#else
    Q_UNUSED(deltaSeconds); Q_UNUSED(width); Q_UNUSED(height); Q_UNUSED(centerOffsetX); Q_UNUSED(centerOffsetY);
    return false;
#endif
}

ID3D11Texture2D* Live2DBridge::nativeTexture() const noexcept {
#if defined(Q_OS_WIN)
    return m->textures && m->target ? m->textures->GetNativeTexture(m->target) : nullptr;
#else
    return nullptr;
#endif
}
QSize Live2DBridge::textureSize() const noexcept { return m->size; }

void Live2DBridge::shutdown() {
#if defined(Q_OS_WIN)
    Q_ASSERT(!m->renderThread || m->onRenderThread());
    m->rememberForDeviceRecovery();
    m->module.Shutdown();
    m->textures.reset(); m->target = 0; m->size = {};
    m->context.Reset(); m->device.Reset();
    m->queuePlaying = false; m->queue.clear(); m->queueIndex = 0;
    m->forceExportFrame = false;
    m->petting.reset();
    if (m->comOwned) { CoUninitialize(); m->comOwned = false; }
    m->renderThread = nullptr;
    m->publish(true);
#endif
}

bool Live2DBridge::beginExportSession(int motionIndex, float fps) {
#if defined(Q_OS_WIN)
    if (!m->onRenderThread() || motionIndex < 0 || static_cast<std::size_t>(motionIndex) >= m->module.MotionNames().size() || !std::isfinite(fps) || fps <= 0) return false;
    if (m->module.ExportSessionActive()) return false;
    m->stopQueue();
    if (!m->module.BeginExportSession(static_cast<std::size_t>(motionIndex), (std::max)(10.0f, fps))) return false;
    m->queuePlaying = false;
    m->forceExportFrame = true;
    m->publish(true);
    return true;
#else
    Q_UNUSED(motionIndex); Q_UNUSED(fps); return false;
#endif
}
bool Live2DBridge::exportSessionSwitchMotion(int motionIndex) {
#if defined(Q_OS_WIN)
    const bool ok = m->onRenderThread() && motionIndex >= 0 && m->module.ExportSessionSwitchMotion(static_cast<std::size_t>(motionIndex));
    if (ok) m->forceExportFrame = true;
    return ok;
#else
    Q_UNUSED(motionIndex); return false;
#endif
}
bool Live2DBridge::renderExportFrame(int motionIndex, bool advance, float advanceSeconds, int width, int height,
                                     float centerOffsetX, float centerOffsetY) {
#if defined(Q_OS_WIN)
    if (!m->onRenderThread() || !m->module.RenderingBackendAvailable() || !m->module.ExportSessionActive()
        || !m->module.HasImportedModel() || width <= 0 || height <= 0) return false;
    if (motionIndex >= 0 && !exportSessionSwitchMotion(motionIndex)) return false;
    if (!std::isfinite(advanceSeconds) || advanceSeconds < 0 || advanceSeconds > 1) return false;
    if (!advance && !m->forceExportFrame && m->target && m->size == QSize(width, height)) return true;
    const bool ok = m->drawTarget(advance ? advanceSeconds : 0.0f, true, width, height, centerOffsetX, centerOffsetY);
    if (ok) { m->forceExportFrame = false; ++m->exportFrameSerial; }
    return ok;
#else
    Q_UNUSED(motionIndex); Q_UNUSED(advance); Q_UNUSED(advanceSeconds); Q_UNUSED(width); Q_UNUSED(height);
    Q_UNUSED(centerOffsetX); Q_UNUSED(centerOffsetY); return false;
#endif
}
void Live2DBridge::endExportSession() {
#if defined(Q_OS_WIN)
    if (!m->onRenderThread()) return;
    m->module.EndExportSession();
    m->forceExportFrame = false;
    m->publish(true);
#endif
}
