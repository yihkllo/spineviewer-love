#ifndef SPINELOVE_RUNTIME_SHARED_SPINE_VIEWPORT_RIG_H_
#define SPINELOVE_RUNTIME_SHARED_SPINE_VIEWPORT_RIG_H_

#include <cstddef>

#include "../sl_gfx_types.h"

class SlViewportRig
{
public:
	virtual ~SlViewportRig() = default;

	virtual void ResetViewScale() = 0;
	virtual void ResetViewScaleAll() = 0;
	virtual void PanByPixels(int x, int y) = 0;
	virtual void PanAllByPixels(int x, int y) = 0;

	virtual SlVec2 BaseSize() const noexcept = 0;
	virtual void SetBaseSize(float width, float height) = 0;
	virtual void ClearBaseSize() = 0;
	virtual SlVec2 SkeletonContentSize() const = 0;

	virtual SlVec2 ViewOffset() const noexcept = 0;
	virtual void SetViewOffset(float x, float y) noexcept = 0;
	virtual SlVec2 ViewOffsetAt(size_t index) const noexcept = 0;
	virtual bool SetViewOffsetAt(size_t index, float x, float y) noexcept = 0;
	virtual bool CenterOpeningPoseInView() noexcept = 0;

	virtual float SkeletonScale() const noexcept = 0;
	virtual void SetSkeletonScale(float scale) = 0;
	virtual float SkeletonScaleAt(size_t index) const noexcept = 0;
	virtual bool SetSkeletonScaleAt(size_t index, float scale) noexcept = 0;

	virtual float CanvasScale() const noexcept = 0;
	virtual void SetCanvasScale(float scale) = 0;

	virtual bool IsMirroredHorizontally() const noexcept = 0;
	virtual void ToggleMirrorX() noexcept = 0;
	virtual int QuarterTurns() const noexcept = 0;
	virtual void RotateClockwise() noexcept = 0;

	virtual bool ResetsViewOnLoad() const noexcept = 0;
	virtual void SetResetViewOnLoad(bool reset) noexcept = 0;
	virtual SlMatrix4 ViewTransform() const noexcept = 0;
	virtual void SetViewportSize(int width, int height) noexcept = 0;
};

#endif
