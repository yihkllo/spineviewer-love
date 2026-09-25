#include "spinelove/interaction_rules.h"

#include <iostream>
#include <limits>

int main()
{
    namespace rules = slqt::interaction;
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (!condition) {
            std::cerr << "FAIL: " << message << '\n';
            ++failures;
        }
    };
    check(!rules::queueReachedEnd(1.0f, 1.0f) && rules::queueReachedEnd(1.001f, 1.0f),
          "queue advances strictly past track end");
    check(!rules::queueReachedEnd(1.0f, 0.0f)
              && !rules::queueReachedEnd(std::numeric_limits<float>::quiet_NaN(), 1.0f),
          "zero-length and invalid clocks cannot spuriously advance queue");
    check(rules::wheelGroupRatio(5.0f, 1, false) == 1.0f
              && rules::layerScaleWithRatio(1.0f, rules::wheelGroupRatio(5.0f, 1, false)) == 1.0f,
          "selected layer at maximum freezes group zoom rather than independently enlarging other layers");
    check(std::abs(rules::wheelScale(1.0f, 1) - 1.05f) < 0.00001f,
          "one wheel notch is exactly original 1.05 factor");
    check(rules::anchorAfterScale(50, 100, 2) == 0,
          "zoom leaves the point beneath the pointer fixed");
    check(rules::slotNameMatches("Body_ARM", "arm") && !rules::slotNameMatches("Body_ARM", ""),
          "slot query folds ASCII and an empty query hides nothing");
    check(!rules::slotNameMatches("\xc3\x84rm", "\xc3\xa4rm"),
          "UTF-8 non-ASCII case remains distinct like original query");
    ReadSlotMeshData mesh;
    mesh.worldVertices = {0, 0, 10, 0, 0, 10};
    mesh.triangles = {0, 1, 2};
    check(rules::slotMeshContains(1, 1, mesh) && !rules::slotMeshContains(9, 9, mesh),
          "mesh picking rejects empty corners inside the bounding rectangle");
    mesh.triangles = {0, 1, 50, 0};
    check(!rules::slotMeshContains(1, 1, mesh), "malformed triangle indices never read beyond mesh arrays");
    const std::vector<std::string> skins{"default", "hat", "dress"};
    check(rules::restoreSkinSelection(skins, {"dress", "hat", "missing"}, true, true, "default")
              == std::vector<std::string>{"hat", "dress"},
          "mixed skin restore intersects names in new catalog order");
    check(rules::restoreSkinSelection(skins, {"dress"}, false, false, "default")
              == std::vector<std::string>{"dress"},
          "single-skin cache restores same name across model changes");
    check(rules::restoreSkinSelection(skins, {"missing"}, true, true, "hat")
              == std::vector<std::string>{"hat"},
          "missing mixed names fall back to current skin before first skin");
    std::cout << "Interaction compatibility checks: " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
