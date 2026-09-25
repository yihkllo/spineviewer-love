#include "spinelove/live2d_bridge.h"
#include "petting_tracker.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QJSEngine>
#include <d3d11.h>
#include <wrl/client.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace {
void Require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
bool Close(const QVariant& value, float expected) { return std::fabs(value.toFloat() - expected) < 0.001f; }
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    const D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0};
    const HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, levels, 1,
        D3D11_SDK_VERSION, &device, nullptr, &context);
    if (FAILED(result)) { std::cerr << "SKIP: D3D11 WARP unavailable\n"; return 77; }
    try {
        slqt::detail::PettingTracker petting;
        Require(!petting.update(true, false, 0, 0, 100), "stationary first head sample triggered petting");
        Require(!petting.update(true, false, 499, 0, 101), "petting threshold fired early");
        Require(petting.update(true, false, 500, 0, 102), "500-pixel head rubbing did not trigger");
        Require(!petting.update(true, false, 0, 0, 103), "petting ignored cooldown");
        Require(!petting.update(true, false, 0, 0, 4102), "stationary polling retriggered petting at cooldown end");
        Require(petting.update(true, false, 1, 0, 4103), "fresh movement after cooldown did not trigger");
        Require(!petting.update(true, true, 1000, 0, 9000), "window dragging triggered petting");
        Require(!petting.update(true, false, 1000, 0, 9001), "reentering head counted off-head travel");
        Require(!petting.update(false, false, 2000, 0, 9002), "off-head motion triggered petting");
        Live2DBridge bridge;
        bridge.command("live2d.open", "missing-fixture.model3.json");
        bridge.command("view.scale", 2.0);
        Require(bridge.initialize(device.Get(), context.Get(), QString::fromUtf8(SL_TEST_SHADER_PATH)), "bridge initialization failed");
        Require(!bridge.render(0, 256, 256), "missing model unexpectedly rendered");
        auto state = bridge.state();
        Require(!state.value("loaded").toBool() && state.value("backendAvailable").toBool(), "unloaded/backend state is inaccurate");
        Require(!state.value("error").toString().isEmpty(), "failed queued import did not report its error");
        Require(Close(state.value("scale"), 2), "commands queued before initialization lost FIFO order");

        std::thread gui([&bridge] {
            for (int i = 0; i < 100; ++i) {
                bridge.command("view.pan", QVariantMap{{"x", 1.0}, {"y", -2.0}});
                bridge.state();
            }
            bridge.command("live2d.effect", QVariantMap{{"key", "breath"}, {"value", false}});
            bridge.command("live2d.gaze", QVariantMap{{"key", "angleX"}, {"value", 17.0}});
            bridge.command("live2d.volume", 0.25);
            bridge.command("playback.speed", 2.5);
        });
        gui.join();
        bridge.render(0, 256, 256);
        state = bridge.state();
        Require(Close(state.value("offsetX"), 100) && Close(state.value("offsetY"), -200), "queued pans were dropped or reordered");
        Require(Close(state.value("timeScale"), 2.5f), "playback speed did not reach the module");
        Require(Close(state.value("voiceVolume"), 0.8f), "unloaded voice state differs from the original module");
        const auto effects = state.value("effects").toMap();
        Require(!effects.value("breath").toBool() && effects.value("physics").toBool() && effects.value("eyeBlink").toBool(), "effect mutation overwrote other effects");
        const auto gaze = state.value("gaze").toMap();
        Require(Close(gaze.value("angleX"), 17) && Close(gaze.value("angleY"), 30), "gaze mutation overwrote other coefficients");

        std::thread scriptProducer([&bridge] {
            QJSEngine engine;
            auto edit = engine.evaluate("({ key: 'angleX', value: 12.1 })");
            bridge.command("live2d.gaze", QVariant::fromValue(edit));
            edit.setProperty("value", 18.5);
            bridge.command("live2d.gaze", QVariant::fromValue(edit));
            bridge.command("live2d.resetGaze");
            edit.setProperty("value", 22.25);
            bridge.command("live2d.gaze", QVariant::fromValue(edit));
            edit.setProperty("value", 59.0);
            bridge.command("live2d.effect", QVariant::fromValue(engine.evaluate("({ key: 'gazeFollow', value: false })")));
            bridge.command("live2d.gaze", QVariantMap{
                {"key", QVariant::fromValue(engine.evaluate("'eyeX'"))},
                {"value", QVariant::fromValue(engine.evaluate("0.375"))}});
            bridge.command("live2d.gaze", QVariantHash{
                {"key", QVariant::fromValue(engine.evaluate("'eyeY'"))},
                {"value", QVariant::fromValue(engine.evaluate("-0.625"))}});
            engine.collectGarbage();
        });
        scriptProducer.join();
        bridge.render(0, 256, 256);
        state = bridge.state();
        const auto detachedGaze = state.value("gaze").toMap();
        Require(Close(detachedGaze.value("angleX"), 22.25f) && Close(detachedGaze.value("angleY"), 30),
            "QML gaze payload was not snapshotted before JS mutation/destruction or crossed reset ordering");
        Require(Close(detachedGaze.value("eyeX"), .375f) && Close(detachedGaze.value("eyeY"), -.625f),
            "nested QJSValue command fields escaped producer-side conversion");
        Require(!state.value("effects").toMap().value("gazeFollow").toBool(),
            "QML effect payload retained a dead JS engine");
        bridge.command("view.reset");
        bridge.command("live2d.resetGaze");
        bridge.render(0, 256, 256);
        state = bridge.state();
        Require(Close(state.value("scale"), 1) && Close(state.value("offsetX"), 0) && Close(state.value("offsetY"), 0), "view reset differs from the original module");
        const QVariantMap zoom{{"steps", 1}, {"inverted", false}, {"x", 150}, {"y", 120}, {"originX", 100}, {"originY", 100}};
        bridge.command("view.zoom", zoom);
        bridge.command("view.zoom", zoom);
        bridge.command("export.sync", QVariantMap{{"requestId", 99}});
        bridge.render(0, 256, 256);
        state = bridge.state();
        Require(state.value("exportAckId").toInt() == 99 && state.value("exportRequestSuccess").toBool(), "export sync barrier did not acknowledge preceding commands");
        Require(Close(state.value("scale"), 1.1025f), "two queued wheel steps collapsed onto stale GUI scale");
        Require(Close(state.value("offsetX"), -5.125f) && Close(state.value("offsetY"), -2.05f), "consecutive wheel zoom lost the cursor anchor");
        bridge.command("view.zoom", QVariantMap{{"steps", 1}, {"inverted", true}, {"centerOnly", true}});
        bridge.render(0, 256, 256);
        state = bridge.state();
        Require(Close(state.value("scale"), 1.05f) && Close(state.value("offsetX"), -5.125f), "pet center-only/reverse zoom changed pan");
        Require(!bridge.nativeTexture() && bridge.textureSize().isEmpty(), "unloaded model exposed a fake render texture");
        Require(!bridge.beginExportSession(0, 30), "unloaded model accepted an export");
        bridge.command("export.begin", QVariantMap{{"motionIndex", 0}, {"fps", 30}, {"requestId", 1}});
        bridge.render(1, 256, 256);
        state = bridge.state();
        Require(state.value("exportAckId").toInt() == 1 && !state.value("exportRequestSuccess").toBool(), "failed export request was not acknowledged");
        Require(!state.value("exportError").toString().isEmpty(), "failed export request lost its error");
        bridge.command("export.end", QVariantMap{{"requestId", 2}});
        bridge.render(1, 256, 256);
        state = bridge.state();
        Require(state.value("exportAckId").toInt() == 2 && state.value("exportRequestSuccess").toBool(), "idempotent export end was not acknowledged");
        bridge.command("view.scale", 1.7f);
        Require(bridge.processPendingCommands() && Close(bridge.state().value("scale"), 1.7f), "hidden command processing did not apply view state");
        bridge.command("export.sync", QVariantMap{{"requestId", 3}});
        bridge.command("view.scale", 2.0f);
        Require(!bridge.processPendingCommands() && Close(bridge.state().value("scale"), 1.7f), "hidden command processing crossed an export barrier");
        bridge.render(0, 256, 256);
        Require(bridge.state().value("exportAckId").toInt() == 3 && Close(bridge.state().value("scale"), 1.7f), "export barrier consumed a later view edit");
        Require(bridge.processPendingCommands() && Close(bridge.state().value("scale"), 2.0f), "view edit after barrier was lost");
        bridge.shutdown();
        Require(!bridge.state().value("backendAvailable").toBool(), "shutdown retained backend availability");
        Require(bridge.initialize(device.Get(), context.Get(), QString::fromUtf8(SL_TEST_SHADER_PATH)), "reinitialization failed");
        Require(!bridge.state().value("loaded").toBool() && bridge.state().value("error").toString().isEmpty(), "reinitialization retried a failed model import");
        bridge.shutdown();
    } catch (const std::exception& exception) {
        std::cerr << "FAIL: " << exception.what() << std::endl;
        return 1;
    }
    std::cout << "PASS: bridge commands, D3D11/Cubism initialization and shutdown (no real model corpus)\n";
    return 0;
}
