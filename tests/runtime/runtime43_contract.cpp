#include "runtime_api.h"
#include "runtime_fixture.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace {
using namespace sl_runtime_v2;
using Json = nlohmann::json;
void Require(bool ok, const std::string& message) {
    if (!ok) throw std::runtime_error(message);
}
Frame FrameOf(IRuntime& runtime) {
    Frame frame;
    runtime.BuildFrame(1280, 720, frame);
    for (const auto& draw : frame.draws) {
        Require(draw.textureId != 0, "missing texture");
        Require(draw.indices.size() % 3 == 0, "incomplete triangles");
        for (auto index : draw.indices) Require(index < draw.vertices.size(), "invalid triangle index");
        for (const auto& v : draw.vertices)
            Require(std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.u) && std::isfinite(v.v), "nonfinite geometry");
    }
    return frame;
}
double Area(const Frame& frame) {
    double area = 0;
    for (const auto& draw : frame.draws) for (size_t i = 0; i < draw.indices.size(); i += 3) {
        const auto& a = draw.vertices[draw.indices[i]];
        const auto& b = draw.vertices[draw.indices[i + 1]];
        const auto& c = draw.vertices[draw.indices[i + 2]];
        area += std::fabs((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) * .5;
    }
    return area;
}
void CheckCoordinates() {
    std::vector<Frame> frames;
    for (const auto version : {"4.2", "4.3"}) {
        auto request = sl_test::Fixture(version);
        auto data = Json::parse(request.skeletonData[0]);
        data["bones"][0]["y"] = 23;
        data["bones"][0]["rotation"] = 30;
        request.skeletonData[0] = data.dump();
        auto runtime = std::string(version) == "4.3" ? CreateCpp43Runtime() : CreateCpp42Runtime();
        Require(runtime->Load(request), runtime->LastError());
        frames.push_back(FrameOf(*runtime));
    }
    Require(frames[0].draws.size() == 1 && frames[1].draws.size() == 1, "coordinate fixture missing");
    const auto& expected = frames[0].draws[0].vertices;
    const auto& actual = frames[1].draws[0].vertices;
    Require(actual.size() == expected.size(), "coordinate vertex count mismatch");
    for (size_t i = 0; i < actual.size(); ++i)
        Require(std::fabs(actual[i].x - expected[i].x) < .001 && std::fabs(actual[i].y - expected[i].y) < .001
            && std::fabs(actual[i].u - expected[i].u) < .001 && std::fabs(actual[i].v - expected[i].v) < .001,
            "4.3 coordinate or texture orientation differs from existing renderer");
}
void CheckClipping() {
    auto request = sl_test::Fixture("4.3");
    auto data = Json::parse(request.skeletonData[0]);
    data["slots"].insert(data["slots"].begin(), Json{{"name", "clip"}, {"bone", "root"}, {"attachment", "mask"}});
    auto& clip = data["skins"][0]["attachments"]["clip"]["mask"];
    clip = {{"type", "clipping"}, {"end", "body"}, {"vertexCount", 4},
        {"vertices", {-8, -8, -8, 8, 8, 8, 8, -8}}, {"convex", true}};
    for (bool inverse : {false, true}) {
        std::cout << "Checking inverse clipping=" << inverse << std::endl;
        clip["inverse"] = inverse;
        request.skeletonData[0] = data.dump();
        auto runtime = CreateCpp43Runtime();
        Require(runtime->Load(request), runtime->LastError());
        std::cout << "Clipping loaded" << std::endl;
        const double expected = inverse ? 768 : 256;
        Require(std::fabs(Area(FrameOf(*runtime)) - expected) < .01, "4.3 clipping area mismatch");
    }
}
void CheckSlider() {
    auto request = sl_test::Fixture("4.3");
    auto data = Json::parse(request.skeletonData[0]);
    data["constraints"] = Json::array({{{"type", "slider"}, {"name", "pose"}, {"animation", "move"}, {"time", .5}}});
    request.skeletonData[0] = data.dump();
    auto runtime = CreateCpp43Runtime();
    Require(runtime->Load(request), runtime->LastError());
    std::array<float, 6> bone;
    Require(runtime->ReadBoneTransform("root", bone) && std::fabs(bone[4] - 8) < .01, "slider constraint was not applied");
    Require(!FrameOf(*runtime).draws.empty(), "slider model has no geometry");
}
void CheckSequence() {
    auto request = sl_test::Fixture("4.3");
    request.atlasData[0] = "probe.png\nsize: 64,32\nfilter: Linear,Linear\n"
        "probe0\nbounds: 0,0,32,32\nprobe1\nbounds: 32,0,32,32\n";
    auto data = Json::parse(request.skeletonData[0]);
    data["skins"][0]["attachments"]["body"]["probe"]["sequence"] = {{"count", 2}, {"start", 0}, {"digits", 1}};
    data["animations"]["move"]["attachments"]["default"]["body"]["probe"]["sequence"] =
        Json::array({{{"time", 0}, {"mode", "hold"}, {"index", 0}}, {{"time", .5}, {"mode", "hold"}, {"index", 1}}});
    request.skeletonData[0] = data.dump();
    auto runtime = CreateCpp43Runtime();
    Require(runtime->Load(request), runtime->LastError());
    runtime->StartMotion("move", false);
    runtime->Update(0);
    const auto start = FrameOf(*runtime);
    runtime->Update(.6f);
    const auto next = FrameOf(*runtime);
    Require(start.draws.size() == 1 && next.draws.size() == 1, "sequence geometry missing");
    Require(std::fabs(next.draws[0].vertices[0].u - start.draws[0].vertices[0].u - .5f) < .001,
        "sequence UVs did not advance");
}
void CheckEventsAndSkins() {
    auto request = sl_test::Fixture("4.3", true);
    auto data = Json::parse(request.skeletonData[0]);
    data["events"]["pulse"] = {{"int", 7}, {"float", 1.5}, {"string", "payload"}};
    data["animations"]["move"]["events"] = {{{"time", .25}, {"name", "pulse"}}};
    request.skeletonData[0] = data.dump();
    auto runtime = CreateCpp43Runtime();
    Require(runtime->Load(request), runtime->LastError());
    runtime->EnableMotionCompletions(true);
    runtime->StartMotion("move", false);
    runtime->Update(.5f);
    std::vector<AnimationEvent> events;
    runtime->DrainAnimationEvents(events);
    Require(events.size() == 1 && events[0].intValue == 7 && events[0].stringValue == "payload"
        && std::fabs(events[0].floatValue - 1.5f) < .001f, "event payload lost");
    std::array<float, 6> bone;
    Require(runtime->ReadBoneTransform("root", bone) && std::fabs(bone[4] - 8) < .01, "applied bone pose mismatch");
    const float x = FrameOf(*runtime).draws[0].vertices[0].x;
    runtime->ApplyLook("alternate");
    Require(std::fabs(FrameOf(*runtime).draws[0].vertices[0].x - x - 7) < .01, "skin attachment did not change");
    runtime->ComposeLooks({"default", "alternate"});
    Require(std::fabs(FrameOf(*runtime).draws[0].vertices[0].x - x - 7) < .01, "skin composition mismatch");
    Require(runtime->SetSlotOverride("body", .25f, SlotAttachmentMode::Preserve), "slot override failed");
    Require(std::fabs(FrameOf(*runtime).draws[0].vertices[0].color.a - .25f) < .001, "slot alpha lost");
    runtime->Update(.6f);
    std::vector<AnimationCompletion> completions;
    runtime->DrainMotionCompletions(completions);
    Require(completions.size() == 1 && completions[0].animation == "move", "completion lost");
    Require(runtime->StartMotionWithMix("empty", true, .2f), "mix transition failed");
    runtime->Update(.3f);
    Require(runtime->ReadBoneTransform("root", bone) && std::fabs(bone[4]) < .01, "mix did not restore setup pose");
    Require(runtime->QueueMotion("move", false, .1f), "queue failed");
    runtime->SetSecondaryMotions({"move"}, true);
    runtime->Update(.1f);
    Require(runtime->HoldMotionTrack(1, .4f), "track hold failed");
    Require(runtime->ClearMotionTrack(1, .1f), "track clear failed");
    runtime->Update(.2f);
    FrameOf(*runtime);
}
std::string Read(const char* path) {
    std::ifstream in(path, std::ios::binary);
    Require(bool(in), "cannot read input");
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}
void CheckAsset(const char* skeleton, const char* atlas) {
    LoadRequest request;
    request.atlasData.push_back(Read(atlas));
    request.skeletonData.push_back(Read(skeleton));
    request.textureDirectories.push_back("fixtures/");
    request.binarySkeleton = std::string(skeleton).find(".skel") != std::string::npos;
    auto runtime = CreateCpp43Runtime();
    Require(runtime->Load(request), runtime->LastError());
    size_t draws = 0;
    const auto skins = runtime->LookNames();
    const auto motions = runtime->MotionNames();
    for (const auto& skin : skins) {
        runtime->ApplyLook(skin.c_str());
        draws += FrameOf(*runtime).draws.size();
    }
    for (const auto& motion : motions) {
        runtime->StartMotion(motion.c_str(), true);
        const float delta = std::min(10.f, std::max(1.f, runtime->MotionDuration(motion.c_str()))) / 120.f;
        for (int i = 0; i < 120; ++i) {
            runtime->Update(delta);
            draws += FrameOf(*runtime).draws.size();
        }
    }
    Require(draws > 0, "sample has no drawable frames");
    std::cout << "PASS: skins=" << skins.size() << " motions=" << motions.size() << " draws=" << draws << '\n';
}
}
int main(int argc, char** argv) {
    try {
        if (argc == 3) CheckAsset(argv[1], argv[2]);
        else {
            Require(argc == 1, "expected skeleton and atlas paths");
            CheckCoordinates();
            CheckClipping();
            CheckSlider();
            CheckSequence();
            std::cout << "Checking events and skins" << std::endl;
            CheckEventsAndSkins();
            std::cout << "PASS: 4.3 clipping, events, skins and tracks\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
