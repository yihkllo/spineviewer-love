#include "spinelove/live2d_bridge.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <d3d11.h>
#include <wrl/client.h>
#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace {
using Microsoft::WRL::ComPtr;
void Require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
bool Near(const QVariant& value, float target) { return std::fabs(value.toFloat() - target) < 0.001f; }
struct Device {
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    Device() {
        const D3D_FEATURE_LEVEL level[] = {D3D_FEATURE_LEVEL_11_0};
        Require(SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, level, 1,
            D3D11_SDK_VERSION, &device, nullptr, &context)), "D3D11 WARP creation failed");
    }
};
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    const auto args = app.arguments();
    if (args.size() != 3) { std::cerr << "usage: spinelove_live2d_lifecycle <model3.json> <report.json>\n"; return 2; }
    Live2DBridge bridge;
    const QString shader = QString::fromUtf8(SL_TEST_SHADER_PATH);
    QVariantMap report{{"model", args[1]}, {"passed", false}};
    std::exception_ptr failure;
    QString parameterId, partId;
    float parameterValue = 0;
    std::thread firstOwner([&] {
        try {
            Device native;
            Require(bridge.initialize(native.device.Get(), native.context.Get(), shader), "initial bridge initialization failed");
            bridge.command("live2d.open", args[1]);
            Require(bridge.render(0, 256, 256), "real model did not render");
            auto state = bridge.state();
            const auto parameters = state.value("parameters").toList(), parts = state.value("parts").toList();
            Require(!parameters.empty() && !parts.empty() && state.value("animations").toList().size() >= 3, "fixture needs parameters, parts and three motions");
            const auto p = parameters.front().toMap();
            parameterId = p.value("id").toString(); partId = parts.front().toMap().value("id").toString();
            parameterValue = (p.value("min").toFloat() + p.value("max").toFloat()) * 0.5f;
            for (int i = 0; i < parameters.size(); ++i) {
                const auto parameter = parameters[i].toMap();
                bridge.command("live2d.parameterValue", QVariantMap{{"index", i},
                    {"value", (parameter.value("min").toFloat() + parameter.value("max").toFloat()) * 0.5f}});
            }
            for (int i = 0; i < parts.size(); ++i) bridge.command("live2d.partValue", QVariantMap{{"index", i}, {"value", 0.4f}});
            bridge.command("view.scale", 1.4f); bridge.command("view.offset", QVariantMap{{"x", 12}, {"y", -8}});
            bridge.command("playback.speed", 0.25f); bridge.command("live2d.volume", 0.2f); bridge.command("live2d.loopAll", true);
            bridge.command("live2d.effect", QVariantMap{{"key", "physics"}, {"value", false}});
            bridge.command("live2d.gaze", QVariantMap{{"key", "angleX"}, {"value", 19}});
            bridge.command("queue.add", 0); bridge.command("queue.add", 0); bridge.command("queue.play");
            bridge.render(0.05f, 256, 256);
            Require(bridge.state().value("queuePlaying").toBool(), "queue did not start");
            Require(!bridge.beginExportSession(99999, 30) && bridge.state().value("queuePlaying").toBool(), "invalid direct export disrupted the queue");
            auto* texture = bridge.nativeTexture();
            bridge.command("live2d.suspend");
            Require(bridge.processPendingCommands(), "hidden-scene command processing failed");
            Require(!bridge.state().value("queuePlaying").toBool() && bridge.nativeTexture() == texture, "suspend did not stop the queue without reallocating/drawing");
            bridge.command("queue.play"); bridge.processPendingCommands();
            report["beforeDeviceLoss"] = bridge.state();
            bridge.command("view.scale", 1.55f);
            bridge.shutdown();
            Require(!bridge.state().value("backendAvailable").toBool(), "shutdown kept the old backend active");
        } catch (...) { failure = std::current_exception(); bridge.shutdown(); }
    });
    firstOwner.join();
    if (!failure) {
        std::thread secondOwner([&] {
            try {
                Device native;
                Require(bridge.initialize(native.device.Get(), native.context.Get(), shader), "new-device bridge initialization failed");
                auto state = bridge.state();
                Require(state.value("loaded").toBool() && state.value("live2dRecoveryCount").toInt() == 1, "model was not recovered on the new device/thread");
                Require(Near(state.value("scale"), 1.4f), "recovery did not precede pending GUI edits");
                Require(bridge.render(0, 256, 256), "recovered model did not render");
                state = bridge.state();
                Require(Near(state.value("scale"), 1.55f) && Near(state.value("timeScale"), 0.25f) && Near(state.value("voiceVolume"), 0.2f), "recovery lost view/speed/audio settings");
                Require(Near(state.value("offsetX"), 12) && Near(state.value("offsetY"), -8) && state.value("loopAll").toBool(), "recovery lost offset/loop state");
                Require(!state.value("effects").toMap().value("physics").toBool() && Near(state.value("gaze").toMap().value("angleX"), 19), "recovery lost effect/gaze settings");
                Require(state.value("queue").toList().size() == 2 && state.value("queuePlaying").toBool(), "recovery lost duplicated queue entries or playback");
                const auto recoveredParameter = state.value("parameters").toList().front().toMap();
                const auto recoveredPart = state.value("parts").toList().front().toMap();
                Require(recoveredParameter.value("id").toString() == parameterId && recoveredParameter.value("overridden").toBool() && Near(recoveredParameter.value("value"), parameterValue), "parameter override was not recovered by ID");
                Require(recoveredPart.value("id").toString() == partId && recoveredPart.value("overridden").toBool() && Near(recoveredPart.value("value"), 0.4f), "part override was not recovered by ID");
                for (const auto& value : state.value("parameters").toList()) {
                    const auto p = value.toMap();
                    Require(p.value("overridden").toBool() && Near(p.value("value"), (p.value("min").toFloat() + p.value("max").toFloat()) * 0.5f), "a parameter override was lost during recovery");
                }
                for (const auto& value : state.value("parts").toList()) {
                    const auto p = value.toMap();
                    Require(p.value("overridden").toBool() && Near(p.value("value"), 0.4f), "a part override was lost during recovery");
                }
                report["recoveredParameterCount"] = state.value("parameters").toList().size();
                report["recoveredPartCount"] = state.value("parts").toList().size();
                ComPtr<ID3D11Device> textureDevice; bridge.nativeTexture()->GetDevice(&textureDevice);
                Require(textureDevice.Get() == native.device.Get(), "recovered texture still belongs to the old device");
                bridge.command("animation.play", 0); bridge.command("animation.step", 1); bridge.command("animation.step", 1);
                bridge.processPendingCommands();
                Require(bridge.state().value("currentAnimation").toInt() == 2, "two queued next-motion requests collapsed into one");
                report["afterDeviceRecovery"] = bridge.state();
                bridge.command("export.begin", QVariantMap{{"motionIndex", 0}, {"fps", 30}, {"requestId", 1}});
                bridge.render(0, 256, 256);
                Require(bridge.state().value("queueExporting").toBool(), "export interruption setup failed");
                bridge.shutdown();
                Require(!bridge.state().value("exportRequestSuccess").toBool() && !bridge.state().value("exportError").toString().isEmpty(), "device loss during export was not reported");
                Require(bridge.initialize(native.device.Get(), native.context.Get(), shader), "post-export recovery failed");
                bridge.command("export.step", QVariantMap{{"delta", 1.0 / 30.0}, {"requestId", 2}});
                bridge.render(0, 256, 256);
                Require(bridge.state().value("exportAckId").toInt() == 2 && !bridge.state().value("exportRequestSuccess").toBool(), "interrupted export silently resumed with a different timeline");
                bridge.command("export.end", QVariantMap{{"requestId", 3}}); bridge.render(0, 256, 256);
                bridge.command("live2d.clear"); bridge.processPendingCommands(); bridge.shutdown();
                Require(bridge.initialize(native.device.Get(), native.context.Get(), shader) && !bridge.state().value("loaded").toBool(), "explicit clear resurrected a recovery snapshot");
                report["clearRemainsClear"] = true;
                report["interruptedExportRejected"] = true;
                report["passed"] = true;
                bridge.shutdown();
            } catch (...) { failure = std::current_exception(); bridge.shutdown(); }
        });
        secondOwner.join();
    }
    if (failure) {
        try { std::rethrow_exception(failure); } catch (const std::exception& error) { report["error"] = QString::fromUtf8(error.what()); std::cerr << error.what() << '\n'; }
    }
    QFile output(args[2]);
    if (!output.open(QIODevice::WriteOnly)) return 2;
    output.write(QJsonDocument(QJsonObject::fromVariantMap(report)).toJson());
    std::cout << (failure ? "FAIL" : "PASS") << ": real Live2D device/thread recovery\n";
    return failure ? 1 : 0;
}
