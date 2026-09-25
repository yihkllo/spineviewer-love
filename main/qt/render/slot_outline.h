#pragma once

#include "spinelove/slot_mesh_data.h"
#include "spinelove/sl_gfx_draw.h"
#include <QImage>
#include <QSize>

namespace slqt {

enum class SlotOutlineMethod { None, AlphaContour, TriangleBoundary, OrderedHull };
struct SlotOutlineResult {
    SlDrawList draws;
    SlotOutlineMethod method = SlotOutlineMethod::None;
    size_t contourPointCount = 0;
};

SlotOutlineResult buildSlotOutline(const ReadSlotMeshData& mesh, const QImage& texture,
    const SlMatrix4& transform, const SlColor& color, float thickness = 3.2f, QSize viewport = {});

}
