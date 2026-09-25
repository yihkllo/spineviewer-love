#include "runtime_api.h"
#include "runtime_fixture.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace sl_runtime_v2;
struct RuntimeCase {
    const char* version;
    RuntimeKind kind;
    std::unique_ptr<IRuntime> (*create)();
};
const RuntimeCase cases[] = {
    {"2.1", RuntimeKind::Cpp21, CreateCpp21Runtime}, {"3.1", RuntimeKind::Cpp31, CreateCpp31Runtime},
    {"3.4", RuntimeKind::Cpp34, CreateCpp34Runtime}, {"3.5", RuntimeKind::Cpp35, CreateCpp35Runtime},
    {"3.6", RuntimeKind::Cpp36, CreateCpp36Runtime}, {"3.7", RuntimeKind::Cpp37, CreateCpp37Runtime},
    {"3.8", RuntimeKind::Cpp38, CreateCpp38Runtime}, {"4.0", RuntimeKind::Cpp40, CreateCpp40Runtime},
    {"4.1", RuntimeKind::Cpp41, CreateCpp41Runtime}, {"4.2", RuntimeKind::Cpp42, CreateCpp42Runtime},
};

void Require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

Frame ReadFrame(IRuntime& runtime) {
    Frame frame;
    runtime.BuildFrame(1280, 720, frame);
    Require(frame.width == 1280 && frame.height == 720, "viewport dimensions were lost");
    Require(frame.draws.size() == 1, "expected one drawable region");
    const DrawCommand& draw = frame.draws.front();
    Require(draw.slotName == "body", "slot identity was lost");
    Require(draw.textureId != 0, "texture identity was lost");
    Require(draw.vertices.size() == 4 && draw.indices.size() == 6, "invalid region geometry");
    for (const auto& vertex : draw.vertices) {
        Require(std::isfinite(vertex.x) && std::isfinite(vertex.y) &&
            std::isfinite(vertex.u) && std::isfinite(vertex.v), "non-finite geometry");
    }
    for (const auto index : draw.indices) Require(index < draw.vertices.size(), "index out of bounds");
    return frame;
}

void Exercise(const RuntimeCase& entry, IRuntime& runtime) {
    const auto info = runtime.Info();
    Require(info.kind == entry.kind && info.versionPrefix && std::string(info.versionPrefix) == entry.version,
        "factory linked to a different Spine version");
    const auto request = sl_test::Fixture(entry.version);
    Require(runtime.Load(request), "memory load failed: " + runtime.LastError());
    Require(runtime.HasSkeleton(), "loaded skeleton unavailable");
    Require(runtime.TextureInfos().size() == 1, "atlas texture enumeration failed");
    Require(runtime.TextureInfos().front().path == "fixtures/probe.png", "texture path changed");
    Require(runtime.LookNames().size() == 1 && runtime.LookNames().front() == "default", "skin enumeration failed");
    Require(runtime.SlotCatalog().size() == 1 && runtime.SlotCatalog().front() == "body", "slot enumeration failed");
    Require(runtime.MotionNames().size() == 2, "animation enumeration failed");
    Require(std::fabs(runtime.MotionDuration("move") - 1.0f) < 0.001f, "motion duration differs");
    runtime.ApplyLook("default");
    runtime.StartMotion("move", false);
    runtime.Update(0.0f);
    const Frame start = ReadFrame(runtime);
    runtime.Update(0.5f);
    const Frame middle = ReadFrame(runtime);
    Require(std::fabs(middle.draws[0].vertices[0].x - start.draws[0].vertices[0].x - 8.0f) < 0.01f,
        "translation timeline did not advance by the expected amount");

    if (entry.kind == RuntimeKind::Cpp34) {
        runtime.StartMotion("empty", true);
        runtime.Update(1.0f / 60.0f);
        const Frame cleared = ReadFrame(runtime);
        auto fresh = entry.create();
        Require(fresh->Load(request), "clean comparison load failed");
        fresh->StartMotion("empty", true);
        fresh->Update(1.0f / 60.0f);
        const Frame expected = ReadFrame(*fresh);
        for (std::size_t i = 0; i < 4; ++i)
            Require(std::fabs(cleared.draws[0].vertices[i].x - expected.draws[0].vertices[i].x) < 0.001f,
                "3.4 empty animation retained the previous pose");
    }
    runtime.Clear();
    Require(!runtime.HasSkeleton() && runtime.TextureInfos().empty(), "Clear retained model state");
    Frame frame = middle;
    runtime.BuildFrame(640, 480, frame);
    Require(frame.draws.empty(), "Clear retained drawable commands");
    Require(runtime.Load(request), "reload failed: " + runtime.LastError());
    ReadFrame(runtime);

    auto noSlash = request;
    noSlash.textureDirectories.front() = "fixtures";
    Require(runtime.Load(noSlash), "texture-root load failed");
    std::string expectedPath = "fixtures/probe.png";
#if defined(_WIN32)
    if (entry.kind == RuntimeKind::Cpp21) expectedPath = "fixtures\\probe.png";
#endif
    Require(runtime.TextureInfos().front().path == expectedPath, "platform texture separator is incorrect");
}
}

int main(int argc, char** argv) {
    if (argc > 2) return 2;
    std::vector<std::unique_ptr<IRuntime>> live;
    try {
        for (const auto& entry : cases) {
            if (argc == 2 && entry.version != std::string(argv[1])) continue;
            std::cout << "Checking Spine " << entry.version << std::endl;
            live.push_back(entry.create());
            Require(live.back() != nullptr, "factory returned null");
            Exercise(entry, *live.back());
        }
        Require(!live.empty(), "unknown Spine version");
        for (auto& runtime : live) ReadFrame(*runtime);
    } catch (const std::exception& exception) {
        std::cerr << "FAIL: " << exception.what() << std::endl;
        return 1;
    }
    std::cout << "PASS: " << live.size() << " runtime(s)" << std::endl;
    return 0;
}

