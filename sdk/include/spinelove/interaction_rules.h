#pragma once

#include "spinelove/slot_mesh_data.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>
#include <vector>

namespace slqt::interaction {

inline bool queueReachedEnd(float track, float end) noexcept
{
    return end > 0.0f && std::isgreater(track, end);
}

inline float wheelScale(float scale, int steps, bool inverted = false,
                        float minimum = 0.1f, float maximum = 5.0f) noexcept
{
    if (steps == 0)
        return scale;
    const float multiplier = std::pow(1.05f, std::abs(static_cast<float>(steps)));
    const bool up = (steps > 0) != inverted;
    return std::clamp(up ? scale * multiplier : scale / multiplier, minimum, maximum);
}

inline float wheelGroupRatio(float activeScale, int steps, bool inverted) noexcept
{
    return activeScale > 0.0f ? wheelScale(activeScale, steps, inverted) / activeScale : 1.0f;
}

inline float layerScaleWithRatio(float oldScale, float ratio) noexcept
{
    return std::clamp(oldScale * ratio, 0.1f, 5.0f);
}

inline float anchorAfterScale(float anchor, float pointer, float ratio) noexcept
{
    return anchor - (pointer - anchor) * (ratio - 1.0f);
}

inline std::string foldAscii(std::string_view text)
{
    std::string folded(text);
    for (char& ch : folded) {
        if (ch >= 'A' && ch <= 'Z')
            ch = static_cast<char>(ch + ('a' - 'A'));
    }
    return folded;
}

inline bool slotNameMatches(std::string_view name, std::string_view query)
{
    return !query.empty() && foldAscii(name).find(foldAscii(query)) != std::string::npos;
}

inline bool pointInTriangle(float px, float py, float ax, float ay,
                            float bx, float by, float cx, float cy) noexcept
{
    const auto cross = [](float x, float y, float a, float b, float c, float d) {
        return (x - c) * (b - d) - (a - c) * (y - d);
    };
    const float d1 = cross(px, py, ax, ay, bx, by);
    const float d2 = cross(px, py, bx, by, cx, cy);
    const float d3 = cross(px, py, cx, cy, ax, ay);
    return !((d1 < 0 || d2 < 0 || d3 < 0) && (d1 > 0 || d2 > 0 || d3 > 0));
}

inline bool slotMeshContains(float x, float y, const ReadSlotMeshData& mesh) noexcept
{
    for (size_t triangle = 0; triangle + 2 < mesh.triangles.size(); triangle += 3) {
        const size_t a = size_t(mesh.triangles[triangle]) * 2;
        const size_t b = size_t(mesh.triangles[triangle + 1]) * 2;
        const size_t c = size_t(mesh.triangles[triangle + 2]) * 2;
        if (a + 1 >= mesh.worldVertices.size() || b + 1 >= mesh.worldVertices.size() || c + 1 >= mesh.worldVertices.size())
            continue;
        if (pointInTriangle(x, y, mesh.worldVertices[a], mesh.worldVertices[a + 1],
                            mesh.worldVertices[b], mesh.worldVertices[b + 1],
                            mesh.worldVertices[c], mesh.worldVertices[c + 1]))
            return true;
    }
    return false;
}

inline std::vector<std::string> restoreSkinSelection(
    const std::vector<std::string>& names, const std::vector<std::string>& cachedSelection,
    bool mixMode, bool previouslyMixed, const std::string& activeName)
{
    const auto present = [&names](const std::string& name) {
        return std::find(names.begin(), names.end(), name) != names.end();
    };
    std::vector<std::string> result;
    if (mixMode && previouslyMixed) {
        for (const auto& name : names) {
            if (std::find(cachedSelection.begin(), cachedSelection.end(), name) != cachedSelection.end())
                result.push_back(name);
        }
    } else if (!mixMode && !previouslyMixed && !cachedSelection.empty() && present(cachedSelection.front())) {
        result.push_back(cachedSelection.front());
    }
    if (result.empty() && !names.empty())
        result.push_back(present(activeName) ? activeName : names.front());
    return result;
}

}
