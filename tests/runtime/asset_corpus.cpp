#include "spinelove/live2d_bridge.h"
#include "spinelove/spine_runtime_registry.h"
#include "spinelove/sl_skeleton_probe.h"
#include "render_d3d11/d3d11_renderer.h"
#include "live2d/live2d_module.h"
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QSaveFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QCryptographicHash>
#include <d3d11.h>
#include <wrl/client.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <stdexcept>

namespace {
using Microsoft::WRL::ComPtr;
void Require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
QByteArray Read(const QString& path) {
    QFile input(path);
    Require(input.open(QIODevice::ReadOnly), "cannot read asset");
    return input.readAll();
}
QJsonObject Pixels(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture, QByteArray* rgba = nullptr) {
    Require(texture != nullptr, "no GPU texture");
    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);
    const UINT width = desc.Width, height = desc.Height;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;
    ComPtr<ID3D11Texture2D> staging;
    Require(SUCCEEDED(device->CreateTexture2D(&desc, nullptr, &staging)), "GPU readback allocation failed");
    context->CopyResource(staging.Get(), texture);
    D3D11_MAPPED_SUBRESOURCE mapped;
    Require(SUCCEEDED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)), "GPU readback failed");
    std::size_t opaque = 0;
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (rgba) rgba->clear();
    for (UINT y = 0; y < height; ++y) {
        const char* row = static_cast<const char*>(mapped.pData) + y * mapped.RowPitch;
        hash.addData(QByteArrayView(row, static_cast<qsizetype>(width) * 4));
        if (rgba) rgba->append(row, static_cast<qsizetype>(width) * 4);
        for (UINT x = 0; x < width; ++x)
            if (static_cast<unsigned char>(row[x * 4 + 3]) > 0) ++opaque;
    }
    context->Unmap(staging.Get(), 0);
    return {{"width", static_cast<int>(width)}, {"height", static_cast<int>(height)},
        {"nontransparentPixels", static_cast<qint64>(opaque)}, {"rgbaSha256", QString::fromLatin1(hash.result().toHex())}};
}

QJsonObject Difference(const QByteArray& first, const QByteArray& second) {
    if (first.size() != second.size()) return {{"sizeMismatch", true}};
    qint64 changedChannels = 0, alphaChangedChannels = 0, totalDelta = 0;
    int maximumDelta = 0;
    for (qsizetype i = 0; i < first.size(); ++i) {
        const int delta = std::abs(static_cast<int>(static_cast<unsigned char>(first[i])) - static_cast<int>(static_cast<unsigned char>(second[i])));
        changedChannels += delta != 0; totalDelta += delta; maximumDelta = (std::max)(maximumDelta, delta);
        if (i % 4 == 3 && delta != 0) ++alphaChangedChannels;
    }
    return {{"changedChannels", changedChannels}, {"alphaChangedChannels", alphaChangedChannels}, {"totalChannels", static_cast<qint64>(first.size())},
        {"maximumChannelDelta", maximumDelta}, {"meanChannelDelta", first.isEmpty() ? 0.0 : static_cast<double>(totalDelta) / first.size()}};
}

QJsonObject CheckOriginalLive2D(const QString& path, ID3D11Device* device, ID3D11DeviceContext* context, sl_d3d11::D3D11Renderer& renderer) {
    QJsonObject record{{"type", "live2d-original-module"}, {"path", path}, {"passed", false}};
    live2d::Live2DModule module;
    SlTextureId target = 0;
    try {
        Require(module.Initialize(device, context, &renderer), "original module initialization failed");
        if (!module.ImportModel(path.toStdWString())) throw std::runtime_error(module.LastError());
        module.SetVoiceVolume(0);
        target = renderer.CreateRenderTarget(256, 256);
        const auto draw = [&](float delta) {
            renderer.BeginRenderTarget(target, SlVec4(0, 0, 0, 0));
            Require(module.TickAndRender(delta, 256, 256, 0, 0, true), "original module draw failed");
            renderer.EndFrame();
        };
        draw(0);
        record.insert("motionCount", static_cast<int>(module.MotionNames().size()));
        record.insert("initialPixels", Pixels(device, context, renderer.GetNativeTexture(target)));
        live2d::RenderBounds bounds;
        if (module.QueryLastRenderedBounds(bounds)) record.insert("initialBounds", QJsonObject{{"x", bounds.x}, {"y", bounds.y}, {"width", bounds.width}, {"height", bounds.height}});
        const auto repeatedExport = [&] {
            QByteArray firstRgba, secondRgba;
            Require(module.BeginExportSession(0, 30), "original begin export failed");
            draw(0.0f);
            const auto first = Pixels(device, context, renderer.GetNativeTexture(target), &firstRgba);
            module.EndExportSession();
            Require(module.BeginExportSession(0, 30), "original second export failed");
            draw(0.0f);
            const auto second = Pixels(device, context, renderer.GetNativeTexture(target), &secondRgba);
            module.EndExportSession();
            return QJsonObject{{"identical", firstRgba == secondRgba}, {"first", first}, {"second", second}, {"difference", Difference(firstRgba, secondRgba)}};
        };
        if (!module.MotionNames().empty()) record.insert("freshRepeatedExport", repeatedExport());
        qint64 maxPixels = 0;
        QJsonArray samples;
        for (std::size_t i = 0; i < module.MotionNames().size(); ++i) {
            module.PlayMotion(i);
            for (int step = 0; step < 10; ++step) draw(0.1f);
            const auto pixels = Pixels(device, context, renderer.GetNativeTexture(target));
            maxPixels = (std::max)(maxPixels, pixels.value("nontransparentPixels").toInteger());
            samples.push_back(QJsonObject{{"name", QString::fromUtf8(module.MotionNames()[i].c_str())}, {"nontransparentPixels", pixels.value("nontransparentPixels")}});
        }
        record.insert("oneSecondMotionSamples", samples);
        record.insert("maxNontransparentPixels", maxPixels);
        if (!module.MotionNames().empty()) record.insert("afterPlaybackRepeatedExport", repeatedExport());
        if (!module.MotionNames().empty()) {
            std::size_t idleIndex = 0;
            for (std::size_t i = 0; i < module.MotionNames().size(); ++i)
                if (QString::fromUtf8(module.MotionNames()[i].c_str()).startsWith("idle /", Qt::CaseInsensitive)) { idleIndex = i; break; }
            const auto sequence = [&](float historyStep) {
                module.SetTimeScale(0.25f); module.PlayMotion(idleIndex);
                for (int i = 0; i < 40; ++i) draw(historyStep);
                Require(module.BeginExportSession(idleIndex, 2), "original sequence begin failed");
                std::vector<QByteArray> frames;
                for (int i = 0; i < 12; ++i) {
                    draw(i == 0 ? 0.0f : 0.5f);
                    frames.emplace_back();
                    Pixels(device, context, renderer.GetNativeTexture(target), &frames.back());
                }
                module.EndExportSession();
                return frames;
            };
            const auto firstHistory = sequence(0.013f);
            const auto secondHistory = sequence(0.017f);
            QJsonArray historyFrames;
            for (std::size_t i = 0; i < firstHistory.size(); ++i)
                historyFrames.push_back(QJsonObject{{"frame", static_cast<int>(i + 1)}, {"difference", Difference(firstHistory[i], secondHistory[i])}});
            record.insert("originalSequenceHistoryProbe", QJsonObject{{"motion", QString::fromUtf8(module.MotionNames()[idleIndex].c_str())},
                {"fps", 2}, {"firstHistoryWallSeconds", 0.52}, {"secondHistoryWallSeconds", 0.68}, {"frames", historyFrames}});
        }
        module.SetModelScale(0.1f);
        draw(0);
        record.insert("minimumScalePixels", Pixels(device, context, renderer.GetNativeTexture(target)));
        if (module.QueryLastRenderedBounds(bounds)) record.insert("minimumScaleBounds", QJsonObject{{"x", bounds.x}, {"y", bounds.y}, {"width", bounds.width}, {"height", bounds.height}});
        Require(maxPixels > 0, "original module also produced transparent frames");
        record.insert("passed", true);
    } catch (const std::exception& error) { record.insert("error", QString::fromUtf8(error.what())); }
    module.Shutdown();
    if (target) renderer.ReleaseTexture(target);
    return record;
}

class TextureHost final : public SlSceneRenderer {
public:
    explicit TextureHost(sl_d3d11::D3D11Renderer& value) : renderer(value) {}
    ~TextureHost() { for (auto id : owned) renderer.ReleaseTexture(id); }
    SlTextureId LoadTextureUtf8(const char* path, bool pma) override {
        const auto wide = QString::fromUtf8(path).toStdWString();
        const auto id = renderer.LoadTexture(wide.c_str(), pma);
        if (id) owned.insert(id); else ++failedTextures;
        return id;
    }
    void ReleaseTexture(SlTextureId id) noexcept override { renderer.ReleaseTexture(id); owned.erase(id); }
    void Submit(const SlDrawList& commands, const std::unordered_map<std::uint64_t, SlTextureId>& textures) override {
        for (const auto& command : commands.commands) {
            ++draws;
            vertices += command.vertices.size();
            for (const auto& vertex : command.vertices)
                Require(std::isfinite(vertex.pos.x) && std::isfinite(vertex.pos.y), "non-finite skeleton vertex");
            for (auto index : command.indices) Require(index < command.vertices.size(), "invalid skeleton index");
        }
        renderer.Submit(commands, textures);
    }
    sl_d3d11::D3D11Renderer& renderer;
    std::set<SlTextureId> owned;
    std::size_t draws = 0, vertices = 0;
    int failedTextures = 0;
};

QJsonObject CheckSpine(const QString& path, const QString& version, ID3D11Device* device, ID3D11DeviceContext* context,
    sl_d3d11::D3D11Renderer& renderer) {
    const QString atlas = QFileInfo(path).dir().filePath(QFileInfo(path).completeBaseName() + ".atlas");
    QJsonObject record{{"type", "spine"}, {"path", path}, {"version", version}, {"atlas", atlas}, {"passed", false}};
    try {
        Require(QFileInfo::exists(atlas), "matching atlas is missing");
        SlRuntimeHub hub;
        const auto lane = hub.LaneForVersionText(version.toUtf8().constData());
        Require(hub.ActivateLane(lane), "unsupported runtime version");
        SlMemoryBundleRequest bundle;
        bundle.atlasText.push_back(Read(atlas).toStdString());
        bundle.skeletonBytes.push_back(Read(path).toStdString());
        bundle.textureRoots.push_back(QFileInfo(path).absolutePath().toUtf8().toStdString() + "/");
        bundle.binarySkeleton = path.endsWith(".skel", Qt::CaseInsensitive);
        auto* player = hub.CurrentRuntime();
        if (!player->LoadBundleFromMemory(bundle)) throw std::runtime_error(player->LastRuntimeIssue());
        const auto names = player->MotionNames();
        const auto skins = player->LookNames();
        record.insert("motionCount", static_cast<int>(names.size()));
        record.insert("skinCount", static_cast<int>(skins.size()));
        record.insert("slotCount", static_cast<int>(player->SlotCatalog().size()));
        player->SetViewportSize(512, 512);
        player->TickPlayback(0);
        const auto content = player->SkeletonContentSize();
        const float span = (std::max)(content.x, content.y);
        if (std::isfinite(span) && span > 0) player->SetSkeletonScale(450.0f / span);
        player->CenterOpeningPoseInView();
        const auto target = renderer.CreateRenderTarget(512, 512);
        Require(target != 0, "Spine render target allocation failed");
        TextureHost host(renderer);
        QJsonArray motionChecks;
        qint64 maxPixels = 0;
        for (std::size_t i = 0; i < (std::max)(std::size_t(1), names.size()); ++i) {
            if (!names.empty()) player->PlayMotionByIndex(i);
            player->TickPlayback(1.0f / 30.0f);
            renderer.BeginRenderTarget(target, SlVec4(0, 0, 0, 0));
            const auto before = host.draws;
            Require(hub.RenderCurrentRuntime(host), "Spine renderer submitted no loaded layer");
            renderer.EndFrame();
            const auto pixels = Pixels(device, context, renderer.GetNativeTexture(target));
            maxPixels = (std::max)(maxPixels, pixels.value("nontransparentPixels").toInteger());
            motionChecks.append(QJsonObject{{"name", names.empty() ? QString{} : QString::fromUtf8(names[i].c_str())},
                {"draws", static_cast<qint64>(host.draws - before)}, {"nontransparentPixels", pixels.value("nontransparentPixels")}});
        }
        for (std::size_t i = 0; i < skins.size(); ++i) { player->ApplyLookByIndex(i); player->TickPlayback(0); }
        renderer.ReleaseTexture(target);
        record.insert("motions", motionChecks);
        record.insert("maxNontransparentPixels", maxPixels);
        record.insert("failedTextures", host.failedTextures);
        record.insert("drawsChecked", static_cast<qint64>(host.draws));
        record.insert("verticesChecked", static_cast<qint64>(host.vertices));
        Require(host.failedTextures == 0, "one or more Spine textures could not load");
        Require(maxPixels > 0, "all Spine motion samples were transparent");
        record.insert("passed", true);
    } catch (const std::exception& error) { record.insert("error", QString::fromUtf8(error.what())); }
    return record;
}

QJsonObject CheckLive2D(const QString& path, ID3D11Device* device, ID3D11DeviceContext* context, const QString& shader) {
    QJsonObject record{{"type", "live2d"}, {"path", path}, {"passed", false}};
    Live2DBridge bridge;
    try {
        Require(bridge.initialize(device, context, shader), "Live2D initialization failed");
        bridge.command("live2d.open", path);
        Require(bridge.render(0, 512, 512), "Live2D initial rendering failed");
        auto state = bridge.state();
        if (!state.value("loaded").toBool()) throw std::runtime_error(state.value("error").toString().toStdString());
        record.insert("initialPixels", Pixels(device, context, bridge.nativeTexture()));
        bridge.command("live2d.volume", 0.0);
        const auto motions = state.value("animations").toList();
        const auto expressions = state.value("expressions").toList();
        record.insert("motionCount", motions.size());
        record.insert("expressionCount", expressions.size());
        record.insert("parameterCount", state.value("parameters").toList().size());
        record.insert("partCount", state.value("parts").toList().size());
        QJsonArray motionChecks;
        qint64 maxPixels = record.value("initialPixels").toObject().value("nontransparentPixels").toInteger();
        for (int i = 0; i < motions.size(); ++i) {
            bridge.command("animation.play", i);
            Require(bridge.render(1.0f / 30.0f, 512, 512), "Live2D motion render failed");
            const auto pixels = Pixels(device, context, bridge.nativeTexture());
            maxPixels = (std::max)(maxPixels, pixels.value("nontransparentPixels").toInteger());
            motionChecks.append(QJsonObject{{"name", motions[i].toMap().value("name").toString()},
                {"nontransparentPixels", pixels.value("nontransparentPixels")}});
        }
        record.insert("motions", motionChecks);
        record.insert("maxNontransparentPixels", maxPixels);
        record.insert("renderingPassed", maxPixels > 0);

        for (int i = 0; i < expressions.size(); ++i) {
            bridge.command("live2d.expression", i);
            Require(bridge.render(1.0f / 30.0f, 256, 256), "Live2D expression render failed");
            Require(bridge.state().value("currentExpression").toInt() == i, "expression index was not applied");
        }
        bridge.command("live2d.clearExpression");
        bridge.command("live2d.effect", QVariantMap{{"key", "physics"}, {"value", false}});
        bridge.render(0, 256, 256);
        Require(!bridge.state().value("effects").toMap().value("physics").toBool(), "physics disable failed");
        bridge.command("live2d.effect", QVariantMap{{"key", "physics"}, {"value", true}});
        bridge.command("live2d.gaze", QVariantMap{{"key", "sensitivity"}, {"value", 1.5}});
        bridge.command("live2d.drag", QVariantMap{{"x", 0.25}, {"y", -0.3}});
        for (int i = 0; i < 5; ++i) Require(bridge.render(0.05f, 256, 256), "physics/gaze update failed");
        state = bridge.state();
        const auto parameters = state.value("parameters").toList();
        for (int i = 0; i < parameters.size(); ++i) {
            const auto p = parameters[i].toMap();
            Require(std::isfinite(p.value("value").toDouble()), "non-finite Live2D parameter");
            bridge.command("live2d.parameterValue", QVariantMap{{"index", i}, {"value", (p.value("min").toDouble() + p.value("max").toDouble()) * 0.5}});
        }
        bridge.render(0, 256, 256);
        for (const auto& p : bridge.state().value("parameters").toList()) Require(p.toMap().value("overridden").toBool(), "parameter override was not retained");
        for (int i = 0; i < parameters.size(); ++i) bridge.command("live2d.parameterReset", i);
        const auto parts = bridge.state().value("parts").toList();
        for (int i = 0; i < parts.size(); ++i) bridge.command("live2d.partValue", QVariantMap{{"index", i}, {"value", 0.5}});
        bridge.render(0, 256, 256);
        for (const auto& p : bridge.state().value("parameters").toList()) Require(!p.toMap().value("overridden").toBool(), "parameter reset retained override");
        for (const auto& p : bridge.state().value("parts").toList()) Require(p.toMap().value("overridden").toBool(), "part override was not retained");
        for (int i = 0; i < parts.size(); ++i) bridge.command("live2d.partReset", i);
        bridge.command("live2d.endDrag");
        bridge.command("live2d.resetGaze");
        bridge.render(0, 256, 256);
        record.insert("allParameterAndPartOverridesReset", true);
        record.insert("physicsAndGazeCommands", true);

        if (!motions.empty()) {
            bridge.command("queue.add", 0); bridge.command("queue.add", 0); bridge.command("queue.play");
            bridge.command("queue.add", 0);
            bridge.command("playback.speed", 5.0);
            bridge.render(0, 128, 128);
            Require(bridge.state().value("queue").toList().size() == 2 && bridge.state().value("queuePlaying").toBool(), "queue duplicates/lock failed");
            bool advanced = false;
            for (int i = 0; i < 160; ++i) {
                bridge.render(0.1f, 128, 128);
                if (bridge.state().value("queueIndex").toInt() == 1) { advanced = true; break; }
            }
            record.insert("queueAdvancedToDuplicate", advanced);
            bridge.command("queue.stop"); bridge.command("queue.clear"); bridge.command("playback.speed", 1.0);
            bridge.render(0, 256, 256);
            Require(!bridge.state().value("queuePlaying").toBool() && bridge.state().value("queue").toList().empty(), "queue stop/clear failed");
            Require(bridge.beginExportSession(0, 30), "deterministic export begin failed");
            QByteArray firstRgba, secondRgba;
            Require(bridge.render(0.0f, 256, 256), "export frame rendering failed");
            const auto first = Pixels(device, context, bridge.nativeTexture(), &firstRgba);
            bridge.endExportSession();
            Require(bridge.beginExportSession(0, 30), "second export begin failed");
            bridge.render(0.0f, 256, 256);
            const auto second = Pixels(device, context, bridge.nativeTexture(), &secondRgba);
            bridge.endExportSession();
            record.insert("exportFrameRepeatedExactly", first.value("rgbaSha256") == second.value("rgbaSha256"));
            record.insert("exportFrameDifference", Difference(firstRgba, secondRgba));
            record.insert("exportPixels", first);

            bridge.command("animation.play", 0);
            bridge.command("queue.add", 0); bridge.command("queue.add", 0);
            bridge.command("export.sync", QVariantMap{{"requestId", 100}});
            bridge.render(99, 256, 256);
            Require(bridge.state().value("exportAckId").toInt() == 100 && bridge.state().value("exportRequestSuccess").toBool() &&
                bridge.state().value("queue").toList().size() == 2, "export sync observed stale queue state");
            const float beforePan = bridge.state().value("offsetX").toFloat();
            bridge.command("export.begin", QVariantMap{{"motionIndex", 0}, {"fps", 30}, {"requestId", 101}});
            bridge.command("view.pan", QVariantMap{{"x", 10}, {"y", 0}});
            Require(bridge.render(99, 256, 256), "export protocol begin frame failed");
            state = bridge.state();
            Require(state.value("exportAckId").toInt() == 101 && state.value("exportRequestSuccess").toBool(), "export begin ACK missing");
            const auto initialSerial = state.value("exportFrameSerial").toULongLong();
            const auto heldFirst = Pixels(device, context, bridge.nativeTexture());
            bridge.render(99, 256, 256);
            const auto heldSecond = Pixels(device, context, bridge.nativeTexture());
            Require(heldFirst.value("rgbaSha256") == heldSecond.value("rgbaSha256"), "idle Qt redraw advanced an export frame");
            bridge.command("export.step", QVariantMap{{"delta", 1.0 / 30.0}, {"requestId", 102}});
            bridge.render(99, 256, 256);
            state = bridge.state();
            Require(state.value("exportAckId").toInt() == 102 && state.value("exportRequestSuccess").toBool() &&
                state.value("exportFrameSerial").toULongLong() == initialSerial + 1, "fixed step ACK/serial is invalid");
            const auto stepped = Pixels(device, context, bridge.nativeTexture());
            bridge.command("export.step", QVariantMap{{"delta", 1.0 / 30.0}, {"requestId", 102}});
            bridge.render(99, 256, 256);
            Require(bridge.state().value("exportFrameSerial").toULongLong() == initialSerial + 1, "retried export step advanced twice");
            Require(stepped.value("rgbaSha256") == Pixels(device, context, bridge.nativeTexture()).value("rgbaSha256"), "retried export step changed pixels");
            Require(std::fabs(bridge.state().value("offsetX").toFloat() - beforePan) < 0.001f, "ordinary GUI mutation escaped export freeze");
            bridge.command("export.switch", QVariantMap{{"motionIndex", 0}, {"requestId", 103}});
            bridge.render(99, 256, 256);
            Require(bridge.state().value("exportAckId").toInt() == 103 && bridge.state().value("exportRequestSuccess").toBool(), "export switch ACK failed");
            bridge.command("export.end", QVariantMap{{"requestId", 104}});
            bridge.render(99, 256, 256);
            Require(bridge.state().value("exportAckId").toInt() == 104 && !bridge.state().value("queueExporting").toBool(), "export end/recovery ACK failed");
            bridge.render(0, 256, 256);
            Require(std::fabs(bridge.state().value("offsetX").toFloat() - beforePan - 10) < 0.001f, "frozen GUI command was lost after export");
            bridge.command("export.begin", QVariantMap{{"motionIndex", 0}, {"fps", 1}, {"requestId", 105}});
            bridge.render(0, 128, 128);
            Require(bridge.state().value("exportAckId").toInt() == 105 && bridge.state().value("exportRequestSuccess").toBool(), "low-FPS export begin failed");
            bridge.command("export.step", QVariantMap{{"delta", 1.0}, {"requestId", 106}});
            bridge.render(99, 128, 128);
            Require(bridge.state().value("exportAckId").toInt() == 106 && bridge.state().value("exportRequestSuccess").toBool(), "one-second fixed export step failed");
            bridge.command("export.end", QVariantMap{{"requestId", 107}});
            bridge.render(0, 128, 128);
            record.insert("exportTransactionProtocol", true);
            record.insert("oneSecondExportStepAccepted", true);
        }
        record.insert("controlsPassed", true);
        Require(maxPixels > 0, "all Live2D motion samples were transparent; controls still exercised");
        record.insert("passed", true);
    } catch (const std::exception& error) {
        record.insert("error", QString::fromUtf8(error.what()));
        record.insert("bridgeError", bridge.state().value("error").toString());
    }
    bridge.shutdown();
    return record;
}
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    const auto args = app.arguments();
    if (args.size() < 3 || args.size() > 4) { std::cerr << "usage: spinelove_asset_corpus <read-only asset root> <output report.json> [--original-live2d]\n"; return 2; }
    const bool originalLive2D = args.size() == 4 && args[3] == "--original-live2d";
    const QDir root(args[1]);
    if (!root.exists()) return 2;
    const QString sourcePrefix = QDir::cleanPath(root.canonicalPath()) + "/";
    const QString reportPath = QDir::cleanPath(QFileInfo(args[2]).absoluteFilePath());
    if (reportPath.startsWith(sourcePrefix, Qt::CaseInsensitive)) {
        std::cerr << "The report must be outside the read-only source asset tree.\n";
        return 2;
    }
    const QString shader = QString::fromUtf8(SL_TEST_SHADER_PATH);
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    const D3D_FEATURE_LEVEL level[] = {D3D_FEATURE_LEVEL_11_0};
    const HRESULT created = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, level, 1, D3D11_SDK_VERSION, &device, nullptr, &context);
    if (FAILED(created)) { std::cerr << "D3D11 WARP unavailable\n"; return 1; }
    const HRESULT apartment = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    sl_d3d11::D3D11Renderer renderer;
    if (!renderer.Initialize(device.Get(), context.Get(), shader.toStdWString().c_str())) return 1;
    QStringList models, skeletons;
    QMap<QString, QString> versions;
    QDirIterator iterator(root.absolutePath(), {"*.json", "*.skel"}, QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        const QString path = iterator.next();
        if (iterator.fileInfo().isSymLink()) continue;
        if (path.endsWith(".model3.json", Qt::CaseInsensitive)) { models.push_back(path); continue; }
        const auto data = Read(path);
        const auto probe = sl_skeleton_probe::Inspect(reinterpret_cast<const unsigned char*>(data.constData()), data.size());
        if (probe.IsSpineSkeleton()) { skeletons.push_back(path); versions.insert(path, QString::fromUtf8(probe.version.c_str())); }
    }
    models.sort(); skeletons.sort();
    QJsonArray records;
    QJsonObject report{{"assetRoot", root.absolutePath()}, {"timestampUtc", QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
        {"device", "Windows x64 D3D11 WARP"}, {"sourceAssetsModified", false},
        {"spineCount", skeletons.size()}, {"live2dCount", models.size()}, {"records", records}};
    const auto save = [&] {
        report.insert("records", records);
        QSaveFile output(reportPath);
        const auto json = QJsonDocument(report).toJson();
        return output.open(QIODevice::WriteOnly) && output.write(json) == json.size() && output.commit();
    };
    int failures = 0;
    for (const auto& path : skeletons) {
        if (originalLive2D) continue;
        std::cout << "Spine " << root.relativeFilePath(path).toUtf8().constData() << std::endl;
        auto record = CheckSpine(path, versions.value(path), device.Get(), context.Get(), renderer);
        if (!record.value("passed").toBool()) ++failures;
        records.push_back(record); if (!save()) return 2;
    }
    for (const auto& path : models) {
        std::cout << "Live2D " << root.relativeFilePath(path).toUtf8().constData() << std::endl;
        auto record = originalLive2D ? CheckOriginalLive2D(path, device.Get(), context.Get(), renderer) : CheckLive2D(path, device.Get(), context.Get(), shader);
        if (!record.value("passed").toBool()) ++failures;
        records.push_back(record); if (!save()) return 2;
    }
    report.insert("failures", failures); report.insert("complete", true); if (!save()) return 2;
    if (SUCCEEDED(apartment)) CoUninitialize();
    std::cout << "Completed " << records.size() << " assets; failures=" << failures << std::endl;
    return failures ? 1 : 0;
}
