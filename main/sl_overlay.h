#ifndef SPINELOVE_SL_OVERLAY_H_
#define SPINELOVE_SL_OVERLAY_H_

#include "sl_gfx_types.h"

struct ReadSlotMeshData;

struct SlOverlayBox
{
	SlVec4 rect;
	SlMatrix4 transform;
	unsigned int color = 0xffffffffu;
	float thickness = 1.0f;
};

struct SlReadSlotMeshOutlineRequest
{
	const ReadSlotMeshData* meshData = nullptr;
	SlMatrix4 transform;
	unsigned int color = 0xffffffffu;
	float thickness = 1.0f;
};

class IOverlayDrawer
{
public:
	virtual ~IOverlayDrawer() = default;
	virtual void DrawBox(const SlOverlayBox& box) = 0;
	virtual void DrawReadSlotMeshOutline(const SlReadSlotMeshOutlineRequest& request) = 0;
};

#endif
