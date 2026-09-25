#include "spinelove/spine_runtime_registry.h"
#include "spinelove/sl_skeleton_probe.h"
#include "runtime_fixture.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void Require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
bool Close(float a, float b) { return std::fabs(a - b) < 0.01f; }

class CapturingRenderer final : public SlSceneRenderer {
public:
    SlTextureId LoadTextureUtf8(const char* path, bool) override {
        paths.emplace_back(path);
        return ++textureCount;
    }
    void ReleaseTexture(SlTextureId) noexcept override { ++releasedCount; }
    void Submit(const SlDrawList& list, const std::unordered_map<std::uint64_t, SlTextureId>& textures) override {
        for (const auto& command : list.commands) {
            Require(textures.count(command.textureId) == 1, "submitted texture has no backend mapping");
            commands.push_back(command);
        }
    }
    void Begin() { commands.clear(); }
    unsigned textureCount = 0;
    unsigned releasedCount = 0;
    std::vector<std::string> paths;
    std::vector<SlDrawCommand> commands;
};

void CheckVersion(const char* version) {
    const auto fixture = sl_test::Fixture(version);
    const std::string& json = fixture.skeletonData.front();
    const auto probe = sl_skeleton_probe::Inspect(reinterpret_cast<const unsigned char*>(json.data()), json.size());
    Require(probe.IsSpineSkeleton() && probe.version == std::string(version) + ".0", "version probe changed");
    SlRuntimeHub hub;
    Require(hub.RuntimePoolReady(), "runtime pool is incomplete");
    const auto lane = hub.LaneForVersionText(probe.version.c_str());
    Require(lane != SlRuntimeHub::RuntimeLane::Unknown && hub.ActivateLane(lane), "version lane is unavailable");
    Require(hub.LaneForVersionText("9.9") == SlRuntimeHub::RuntimeLane::Unknown, "unknown version accepted");
    auto* player = hub.CurrentRuntime();
    Require(player != nullptr, "player is missing");
    SlMemoryBundleRequest bundle;
    bundle.atlasText = {fixture.atlasData.front(), fixture.atlasData.front()};
    bundle.textureRoots = {"fixtures/", "fixtures/"};
    bundle.skeletonBytes = {json, json};
    Require(player->LoadBundleFromMemory(bundle), "multi-layer memory load failed");
    Require(player->LoadedSkeletonCount() == 2, "multi-layer model count changed");
    Require(player->ChooseSkeleton(0) && !player->ChooseSkeleton(2), "layer selection bounds changed");
    player->SetViewportSize(640, 480);
    player->PlayMotionByName("empty");
    player->TickPlayback(0.0f);
    CapturingRenderer renderer;
    Require(hub.RenderCurrentRuntime(renderer), "generic render failed");
    Require(renderer.commands.size() == 2 && renderer.textureCount == 2, "two-layer render omitted content");
    const float startX = renderer.commands[1].vertices[0].pos.x;
    const float otherX = renderer.commands[0].vertices[0].pos.x;
    const auto offset = player->ViewOffset();
    player->SetViewOffset(offset.x + 17.0f, offset.y);
    renderer.Begin();
    hub.RenderCurrentRuntime(renderer);
    Require(Close(renderer.commands[1].vertices[0].pos.x - startX, 17.0f), "selected-layer pan changed");
    Require(Close(renderer.commands[0].vertices[0].pos.x, otherX), "selected-layer pan affected another layer");
    Require(renderer.textureCount == 2, "unchanged render reloaded textures");

    player->SetSkeletonLayerOpacity(0, 0.25f);
    player->SetSkeletonLayerTint(0, SlColor(0.5f, 1.0f, 1.0f, 0.8f));
    renderer.Begin();
    hub.RenderCurrentRuntime(renderer);
    Require(Close(renderer.commands[1].vertices[0].color.a, 0.2f), "layer opacity/tint alpha changed");
    Require(player->DemoteSkeleton(0) && player->ActiveSkeletonIndex() == 1, "reorder lost active layer");
    Require(Close(player->SkeletonLayerOpacity(1), 0.25f), "reorder lost layer properties");
    Require(player->PromoteSkeleton(1) && player->ActiveSkeletonIndex() == 0, "reverse reorder changed selection");

    player->SetHiddenSlots({"body"});
    renderer.Begin();
    hub.RenderCurrentRuntime(renderer);
    Require(renderer.commands.size() == 1, "slot hiding affected the wrong layer");
    player->SetHiddenSlots({});
    player->SetSkeletonLayerVisible(1, false);
    renderer.Begin();
    hub.RenderCurrentRuntime(renderer);
    Require(renderer.commands.size() == 1, "layer visibility is ignored");
    SlRect bounds;
    Require(hub.QueryLastRenderedBounds(bounds) && bounds.w > 0 && bounds.h > 0, "rendered bounds are missing");

    Require(player->LoadBundleFromMemory(bundle), "reload failed");
    renderer.Begin();
    hub.RenderCurrentRuntime(renderer);
    Require(renderer.releasedCount == 2, "reload did not release the previous texture mappings");

    SlRuntimeHub appendHub;
    Require(appendHub.ActivateLane(lane), "append test lane unavailable");
    auto* appendPlayer = appendHub.CurrentRuntime();
    SlMemoryBundleRequest single;
    single.atlasText = {fixture.atlasData.front()};
    single.skeletonBytes = {json};
    single.textureRoots = {u8"fixtures/\u8D44\u6599/"};
    Require(appendPlayer->LoadBundleFromMemory(single), "initial single-layer memory load failed");
    appendPlayer->SetViewportSize(640, 480);
    appendPlayer->SetViewOffset(13, -29);
    appendPlayer->SetSkeletonScale(1.7f);
    appendPlayer->SetSkeletonLayerOpacity(0, 0.4f);
    appendPlayer->SetHiddenSlots({"body"});
    appendPlayer->PlayMotionByName("move");
    appendPlayer->TickPlayback(0.25f);
    const std::string previousMotion = appendPlayer->ActiveMotionName();
    float beforeTrack = 0, beforeLast = 0, beforeStart = 0, beforeEnd = 0;
    appendPlayer->ReadMotionClock(&beforeTrack, &beforeLast, &beforeStart, &beforeEnd);
    Require(appendPlayer->AddLayerFromMemory(single), "append from memory failed");
    Require(appendPlayer->LoadedSkeletonCount() == 2 && appendPlayer->ActiveSkeletonIndex() == 0, "append reset layer count/selection");
    Require(Close(appendPlayer->ViewOffset().x, 13) && Close(appendPlayer->ViewOffset().y, -29) &&
        Close(appendPlayer->SkeletonScale(), 1.7f) && Close(appendPlayer->SkeletonLayerOpacity(0), 0.4f), "append reset existing transform/opacity");
    Require(appendPlayer->ActiveMotionName() == previousMotion, "append changed the active motion");
    float afterTrack = 0, afterLast = 0, afterStart = 0, afterEnd = 0;
    appendPlayer->ReadMotionClock(&afterTrack, &afterLast, &afterStart, &afterEnd);
    Require(Close(beforeTrack, afterTrack) && Close(beforeLast, afterLast), "append reset existing playback time");
    CapturingRenderer appendRenderer;
    Require(appendHub.RenderCurrentRuntime(appendRenderer), "appended model did not render");
    Require(appendRenderer.commands.size() == 1, "append lost the original layer's hidden-slot rule");
    Require(appendRenderer.paths.size() == 2 && appendRenderer.paths.front() == u8"fixtures/\u8D44\u6599/probe.png", "append corrupted a UTF-8 texture root");
    SlMemoryBundleRequest incomplete;
    Require(!appendPlayer->AddLayerFromMemory(incomplete) && appendPlayer->LoadedSkeletonCount() == 2,
        "failed append cleared existing layers");

    const auto twoLooks = sl_test::Fixture(version, true);
    SlMemoryBundleRequest skinBundle;
    skinBundle.atlasText = {twoLooks.atlasData.front(), twoLooks.atlasData.front()};
    skinBundle.skeletonBytes = {twoLooks.skeletonData.front(), twoLooks.skeletonData.front()};
    skinBundle.textureRoots = {"fixtures/", "fixtures/"};
    SlRuntimeHub skinHub;
    Require(skinHub.ActivateLane(lane), "skin test lane unavailable");
    auto* skinPlayer = skinHub.CurrentRuntime();
    Require(skinPlayer->LoadBundleFromMemory(skinBundle), "two-skin fixture failed to load");
    skinPlayer->SetViewportSize(640, 480);
    for (std::size_t i = 0; i < 2; ++i) {
        skinPlayer->ChooseSkeleton(i); skinPlayer->ApplyLookByName("default"); skinPlayer->PlayMotionByName("empty");
    }
    skinPlayer->TickPlayback(0);
    CapturingRenderer skinRenderer;
    Require(skinHub.RenderCurrentRuntime(skinRenderer) && skinRenderer.commands.size() == 2, "skin baseline did not render both layers");
    const float firstDefaultX = skinRenderer.commands[1].vertices[0].pos.x;
    const float secondDefaultX = skinRenderer.commands[0].vertices[0].pos.x;
    skinPlayer->ChooseSkeleton(0); skinPlayer->ApplyLookByName("alternate"); skinPlayer->TickPlayback(0);
    Require(skinPlayer->ActiveLookName() == "alternate", "selected skin name was not recorded on its layer");
    skinRenderer.Begin(); skinHub.RenderCurrentRuntime(skinRenderer);
    Require(Close(skinRenderer.commands[1].vertices[0].pos.x - firstDefaultX, 7), "alternate skin did not change its layer geometry");
    Require(Close(skinRenderer.commands[0].vertices[0].pos.x, secondDefaultX), "changing one skin modified another layer");
    skinPlayer->ChooseSkeleton(1);
    Require(skinPlayer->ActiveLookName() == "default", "layer selection retained the previous layer's skin name");
    skinPlayer->ApplyLookByName("default"); skinPlayer->ChooseSkeleton(0);
    Require(skinPlayer->ActiveLookName() == "alternate", "returning to a layer lost its skin name");
    skinPlayer->ComposeLooks({"default", "alternate"});
    skinPlayer->ChooseSkeleton(1);
    Require(skinPlayer->ActiveLookName() == "default", "composing one layer changed another layer's skin bookkeeping");
    skinPlayer->ChooseSkeleton(0);
    Require(skinPlayer->ActiveLookName() == "alternate", "composed look name did not survive layer selection");
    Require(skinPlayer->DemoteSkeleton(0) && skinPlayer->ActiveLookName() == "alternate", "layer reorder detached the skin name");
    skinPlayer->ChooseSkeleton(0);
    Require(skinPlayer->ActiveLookName() == "default", "reordered layer reports another layer's skin");
    skinPlayer->ApplyLookByName("alternate"); skinPlayer->PlayMotionByName("empty");
    SlMemoryBundleRequest skinAppend;
    skinAppend.atlasText = {twoLooks.atlasData.front()};
    skinAppend.skeletonBytes = {twoLooks.skeletonData.front()};
    skinAppend.textureRoots = {"fixtures/"};
    Require(skinPlayer->AddLayerFromMemory(skinAppend) && skinPlayer->ChooseSkeleton(2), "skin append failed");
    Require(skinPlayer->ActiveLookName() == "alternate" && skinPlayer->ActiveMotionName() == "empty", "opening skin/motion metadata differs from actual append selection");
}
}

int main() {
    try {
        for (const char* version : {"2.1", "3.1", "3.4", "3.5", "3.6", "3.7", "3.8", "4.0", "4.1", "4.2"}) {
            std::cout << "Checking playback " << version << std::endl;
            CheckVersion(version);
        }
    } catch (const std::exception& exception) {
        std::cerr << "FAIL: " << exception.what() << std::endl;
        return 1;
    }
    std::cout << "PASS: all playback versions" << std::endl;
    return 0;
}
