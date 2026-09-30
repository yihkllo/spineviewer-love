#include "spinelove/live2d_bridge.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QOffscreenSurface>
#include <rhi/qrhi.h>
#if QT_CONFIG(vulkan) && __has_include(<vulkan/vulkan.h>)
#define SL_TEST_VULKAN 1
#include <QVulkanInstance>
#endif
#include <d3d11.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace {
struct Device {
    std::unique_ptr<QOffscreenSurface> surface;
    std::unique_ptr<QRhi> rhi;
};

Device Create(const QString& backend) {
    Device device;
    const QRhi::Flags flags = QRhi::EnableTimestamps;
    if (backend == "d3d11") {
        QRhiD3D11InitParams params;
        device.rhi.reset(QRhi::create(QRhi::D3D11, &params, flags));
    } else if (backend == "d3d12") {
        QRhiD3D12InitParams params;
        device.rhi.reset(QRhi::create(QRhi::D3D12, &params, flags));
    } else if (backend == "opengl") {
        device.surface.reset(QRhiGles2InitParams::newFallbackSurface());
        QRhiGles2InitParams params;
        params.fallbackSurface = device.surface.get();
        device.rhi.reset(QRhi::create(QRhi::OpenGLES2, &params, flags));
    }
#if defined(SL_TEST_VULKAN)
    else if (backend == "vulkan") {
        static QVulkanInstance instance;
        if (!instance.isValid()) {
            instance.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
            instance.create();
        }
        if (instance.isValid()) {
            QRhiVulkanInitParams params;
            params.inst = &instance;
            device.rhi.reset(QRhi::create(QRhi::Vulkan, &params, flags));
        }
    }
#endif
    return device;
}

struct Timing { double cpu = 0, frame = 0, gpu = 0; };

QImage Frame(QRhi* rhi, const std::function<QRhiTexture*(QRhiCommandBuffer*)>& record, Timing* timing = nullptr) {
    QRhiCommandBuffer* cb = nullptr;
    QElapsedTimer total;
    total.start();
    if (rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess) return {};
    QElapsedTimer cpu;
    cpu.start();
    QRhiTexture* texture = record(cb);
    const double cpuMs = cpu.nsecsElapsed() / 1e6;
    QRhiReadbackResult result;
    if (texture) {
        auto* batch = rhi->nextResourceUpdateBatch();
        batch->readBackTexture(QRhiReadbackDescription(texture), &result);
        cb->resourceUpdate(batch);
    }
    rhi->endOffscreenFrame();
    if (timing) { timing->cpu = cpuMs; timing->frame = total.nsecsElapsed() / 1e6; timing->gpu = cb->lastCompletedGpuTime() * 1000.0; }
    if (!texture || result.pixelSize.isEmpty()) return {};
    QImage image(reinterpret_cast<const uchar*>(result.data.constData()), result.pixelSize.width(), result.pixelSize.height(),
                 QImage::Format_RGBA8888_Premultiplied);
    return rhi->isYUpInFramebuffer() ? image.mirrored(false, true) : image.copy();
}

struct Difference { int maximum = 0; double mean = 0; double over = 0; };

Difference Compare(const QImage& a, const QImage& b, QImage* heat) {
    Difference d;
    if (a.size() != b.size() || a.isNull()) { d.maximum = 255; d.over = 1; return d; }
    if (heat) *heat = QImage(a.size(), QImage::Format_RGBA8888);
    double sum = 0;
    qint64 over = 0;
    for (int y = 0; y < a.height(); ++y) {
        const uchar* p = a.constScanLine(y);
        const uchar* q = b.constScanLine(y);
        uchar* h = heat ? heat->scanLine(y) : nullptr;
        for (int x = 0; x < a.width(); ++x) {
            int m = 0;
            for (int c = 0; c < 4; ++c) m = (std::max)(m, std::abs(int(p[x * 4 + c]) - int(q[x * 4 + c])));
            d.maximum = (std::max)(d.maximum, m);
            sum += m;
            if (m > 3) ++over;
            if (h) { const uchar v = uchar((std::min)(255, m * 16)); h[x * 4] = v; h[x * 4 + 1] = v; h[x * 4 + 2] = v; h[x * 4 + 3] = 255; }
        }
    }
    const double count = double(a.width()) * a.height();
    d.mean = sum / count;
    d.over = over / count;
    return d;
}

double Average(std::vector<double> values) {
    if (values.empty()) return 0;
    std::sort(values.begin(), values.end());
    return values[values.size() / 2];
}
}

int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
    QString model = qEnvironmentVariable("SPINELOVE_LIVE2D_FIXTURE");
    QStringList backends{"d3d11", "d3d12", "opengl", "vulkan"};
    QString output;
    int frames = 45, width = 1280, height = 1280, perfFrames = 240, batchSize = 4;
    const QStringList args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        const QString a = args[i];
        if (a == "--model" && i + 1 < args.size()) model = args[++i];
        else if (a == "--backends" && i + 1 < args.size()) backends = args[++i].split(',', Qt::SkipEmptyParts);
        else if (a == "--frames" && i + 1 < args.size()) frames = args[++i].toInt();
        else if (a == "--size" && i + 2 < args.size()) { width = args[++i].toInt(); height = args[++i].toInt(); }
        else if (a == "--perf" && i + 1 < args.size()) perfFrames = args[++i].toInt();
        else if (a == "--out" && i + 1 < args.size()) output = args[++i];
        else if (a == "--batch" && i + 1 < args.size()) batchSize = (std::max)(1, args[++i].toInt());
    }
    if (model.isEmpty() || !QFileInfo::exists(model)) { std::cerr << "SKIP: set SPINELOVE_LIVE2D_FIXTURE or --model\n"; return 77; }
    if (!output.isEmpty()) QDir().mkpath(output);
    const float step = 1.0f / 30.0f;

    Device reference = Create("d3d11");
    if (!reference.rhi) { std::cerr << "SKIP: D3D11 unavailable\n"; return 77; }
    const auto* native = static_cast<const QRhiD3D11NativeHandles*>(reference.rhi->nativeHandles());
    std::vector<QImage> expected;
    std::vector<double> refCpu, refFrame, refGpu;
    {
        Live2DBridge bridge;
        if (!bridge.initialize(static_cast<ID3D11Device*>(native->dev), static_cast<ID3D11DeviceContext*>(native->context), QString::fromUtf8(SL_TEST_SHADER_PATH))) {
            std::cerr << "FAIL: D3D11 bridge\n"; return 1;
        }
        std::unique_ptr<QRhiTexture> wrapped;
        const auto wrap = [&]() -> QRhiTexture* {
            auto* texture = bridge.nativeTexture();
            if (!texture) return nullptr;
            wrapped.reset(reference.rhi->newTexture(QRhiTexture::RGBA8, bridge.textureSize(), 1, QRhiTexture::UsedAsTransferSource));
            return wrapped->createFrom({quint64(reinterpret_cast<quintptr>(texture)), 0}) ? wrapped.get() : nullptr;
        };
        bridge.command("live2d.open", model);
        Frame(reference.rhi.get(), [&](QRhiCommandBuffer* cb) { cb->beginExternal(); bridge.render(0, width, height); cb->endExternal(); return wrap(); });
        if (!bridge.state().value("loaded").toBool()) { std::cerr << "FAIL: " << bridge.state().value("error").toString().toStdString() << "\n"; return 1; }
        for (int i = 0; i < perfFrames; ++i) {
            Timing t;
            Frame(reference.rhi.get(), [&](QRhiCommandBuffer* cb) { cb->beginExternal(); bridge.render(1.0f / 60.0f, width, height); cb->endExternal(); return wrap(); }, &t);
            if (i >= 20) { refCpu.push_back(t.cpu); refFrame.push_back(t.frame); refGpu.push_back(t.gpu); }
        }
        if (!bridge.beginExportSession(0, 30)) { std::cerr << "FAIL: export session\n"; return 1; }
        for (int i = 0; i < frames; ++i)
            expected.push_back(Frame(reference.rhi.get(), [&](QRhiCommandBuffer* cb) {
                cb->beginExternal(); bridge.renderExportFrame(-1, i > 0, step, width, height); cb->endExternal(); return wrap(); }));
        bridge.endExportSession();
        wrapped.reset();
        bridge.shutdown();
    }
    std::cout << "reference d3d11-native: cpu " << Average(refCpu) << " ms, frame " << Average(refFrame) << " ms, gpu " << Average(refGpu) << " ms\n";
    if (!output.isEmpty() && !expected.empty()) expected.front().save(output + "/reference_0.png"), expected.back().save(output + "/reference_last.png");

    int failures = 0;
    for (const QString& name : backends) {
        Device device = Create(name);
        if (!device.rhi) { std::cout << name.toStdString() << ": unavailable\n"; continue; }
        Live2DBridge bridge;
        std::vector<double> cpu, frame, gpu;
        Difference worst;
        double meanTotal = 0;
        {
            if (!bridge.initializeRhi(device.rhi.get())) { std::cout << name.toStdString() << ": init failed " << bridge.state().value("error").toString().toStdString() << "\n"; ++failures; continue; }
            bridge.command("live2d.open", model);
            Frame(device.rhi.get(), [&](QRhiCommandBuffer* cb) { bridge.renderRhi(cb, 0, width, height); return bridge.rhiTexture(); });
            if (!bridge.state().value("loaded").toBool()) { std::cout << name.toStdString() << ": load failed " << bridge.state().value("error").toString().toStdString() << "\n"; ++failures; bridge.shutdown(); continue; }
            for (int i = 0; i < perfFrames; ++i) {
                Timing t;
                Frame(device.rhi.get(), [&](QRhiCommandBuffer* cb) { bridge.renderRhi(cb, 1.0f / 60.0f, width, height); return bridge.rhiTexture(); }, &t);
                if (i >= 20) { cpu.push_back(t.cpu); frame.push_back(t.frame); gpu.push_back(t.gpu); }
            }
            if (!bridge.beginExportSession(0, 30)) { std::cout << name.toStdString() << ": export failed\n"; ++failures; bridge.shutdown(); continue; }
            std::vector<QImage> images;
            for (int first = 0; first < frames; first += batchSize) {
                const int last = (std::min)(frames, first + batchSize);
                std::vector<std::unique_ptr<QRhiReadbackResult>> results;
                QRhiCommandBuffer* cb = nullptr;
                if (device.rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess) break;
                for (int i = first; i < last; ++i) {
                    bridge.renderExportFrameRhi(cb, -1, i > 0, step, width, height, 0, 0, i > first);
                    results.push_back(std::make_unique<QRhiReadbackResult>());
                    auto* batch = device.rhi->nextResourceUpdateBatch();
                    batch->readBackTexture(QRhiReadbackDescription(bridge.rhiTexture()), results.back().get());
                    cb->resourceUpdate(batch);
                }
                device.rhi->endOffscreenFrame();
                for (const auto& result : results) {
                    QImage image(reinterpret_cast<const uchar*>(result->data.constData()), result->pixelSize.width(), result->pixelSize.height(),
                                 QImage::Format_RGBA8888_Premultiplied);
                    images.push_back(device.rhi->isYUpInFramebuffer() ? image.mirrored(false, true) : image.copy());
                }
            }
            for (int i = 0; i < frames && size_t(i) < images.size(); ++i) {
                const QImage& image = images[size_t(i)];
                QImage heat;
                const Difference d = Compare(expected[size_t(i)], image, (!output.isEmpty() && (i == 0 || i == frames - 1)) ? &heat : nullptr);
                if (!output.isEmpty() && (i == 0 || i == frames - 1)) {
                    const QString suffix = i == 0 ? "_0.png" : "_last.png";
                    image.save(output + "/" + name + suffix);
                    heat.save(output + "/" + name + "_diff" + suffix);
                }
                worst.maximum = (std::max)(worst.maximum, d.maximum);
                worst.over = (std::max)(worst.over, d.over);
                meanTotal += d.mean;
            }
            if (images.size() < size_t(frames)) worst.over = 1;
            bridge.endExportSession();
            bridge.shutdown();
        }
        const double mean = frames ? meanTotal / frames : 0;
        const bool pass = worst.over < 0.002 && mean < 0.1;
        failures += pass ? 0 : 1;
        std::cout << name.toStdString() << (pass ? " PASS" : " FAIL") << ": max " << worst.maximum << ", mean " << mean
                  << ", pixels>3 " << worst.over * 100 << "%, cpu " << Average(cpu) << " ms, frame " << Average(frame) << " ms, gpu " << Average(gpu) << " ms\n";
    }
    return failures ? 1 : 0;
}
