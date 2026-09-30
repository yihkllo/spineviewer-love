#pragma once

#include "spinelove/slot_mesh_data.h"
#include "spinelove/sl_gfx_draw.h"
#include <QImage>
#include <QSize>
#include <string>
#include <unordered_map>
#include <vector>

namespace slqt {

enum class SlotOutlineMethod { None, AlphaContour, TriangleBoundary, OrderedHull };
struct SlotOutlineResult {
    SlDrawList draws;
    SlotOutlineMethod method = SlotOutlineMethod::None;
    size_t contourPointCount = 0;
};
struct SlotOutlineAnchor {
    unsigned short a = 0, b = 0, c = 0;
    float wa = 0, wb = 0, wc = 0;
};

SlotOutlineResult buildSlotOutline(const ReadSlotMeshData& mesh, const QImage& texture,
    const SlMatrix4& transform, const SlColor& color, float thickness = 3.2f, QSize viewport = {});

class SlotOutlineCache {
public:
    SlotOutlineResult build(const std::string& slot, const ReadSlotMeshData& mesh, const QImage& texture,
        const SlMatrix4& transform, const SlColor& color, float thickness = 3.2f, QSize viewport = {});
    void clear() { m_entries.clear(); }
    size_t rasterCount() const { return m_rasters; }

private:
    struct Entry {
        bool ready = false, contour = false;
        int textureHandle = -1;
        qint64 textureKey = 0;
        float thickness = 0, spanX = 0, spanY = 0, dot = 0;
        std::vector<float> uvs;
        std::vector<unsigned short> triangles;
        std::vector<SlotOutlineAnchor> anchors;
    };
    std::unordered_map<std::string, Entry> m_entries;
    size_t m_rasters = 0;
};

}
