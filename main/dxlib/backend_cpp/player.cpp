
#include <cmath>
#include "player.h"

CDxLibSpinePlayer::CDxLibSpinePlayer()  = default;
CDxLibSpinePlayer::~CDxLibSpinePlayer() = default;

void CDxLibSpinePlayer::draw()
{
	if (m_drawables.empty()) return;
	auto drawAtIndex = [&](size_t index)
	{
		if (index >= m_drawableStates.size() || !m_drawableStates[index].visible)
			return;
		DxLib::MATRIX mtx = calculateTransformMatrix(index);
		DxLib::SetTransformTo2D(&mtx);
		m_drawables[index]->draw();
		DxLib::ResetTransformTo2D();
	};

	if (m_isDrawOrderReversed)
	{
		for (size_t i = 0; i < m_drawables.size(); ++i)
			drawAtIndex(i);
	}
	else
	{
		for (size_t i = m_drawables.size(); i-- > 0;)
			drawAtIndex(i);
	}

	drawSelectedOutline();
}

DxLib::MATRIX CDxLibSpinePlayer::calculateTransformMatrix() const noexcept
{
	return calculateTransformMatrix(m_selectedDrawableIndex);
}

DxLib::MATRIX CDxLibSpinePlayer::calculateTransformMatrix(size_t index) const noexcept
{
	int sw = m_renderWidth;
	int sh = m_renderHeight;
	if (sw <= 0 || sh <= 0)
		DxLib::GetDrawScreenSize(&sw, &sh);

	const float halfW = m_fBaseSize.x * 0.5f;
	const float halfH = m_fBaseSize.y * 0.5f;

	DxLib::MATRIX m = DxLib::MGetTranslate(DxLib::VGet(-halfW, -halfH, 0.f));

	float skeletonScale = m_fSkeletonScale;
	bool flipX = m_bFlipX;
	int rotationSteps = m_iRotationSteps;
	if (index < m_drawableStates.size())
	{
		skeletonScale = m_drawableStates[index].skeletonScale;
		flipX = m_drawableStates[index].flipX;
		rotationSteps = m_drawableStates[index].rotationSteps;
	}
	const float sx = flipX ? -skeletonScale : skeletonScale;
	m = DxLib::MMult(m, DxLib::MGetScale(DxLib::VGet(sx, skeletonScale, 1.f)));

	constexpr float kHalfPi = 1.5707963267948966f;
	m = DxLib::MMult(m, DxLib::MGetRotZ(kHalfPi * rotationSteps));

	m = DxLib::MMult(m, DxLib::MGetTranslate(DxLib::VGet(sw * 0.5f, sh * 0.5f, 0.f)));

	return m;
}

void CDxLibSpinePlayer::drawSelectedOutline() const
{
}

void CDxLibSpinePlayer::setRenderScreenSize(int width, int height) noexcept
{
	m_renderWidth  = width;
	m_renderHeight = height;
}

DxLib::FLOAT4 CDxLibSpinePlayer::getCurrentBoundingOfSlot(const std::string& slotName) const
{
	if (m_selectedDrawableIndex >= m_drawables.size()) return {};

	bool hit = false;
	DxLib::FLOAT4 rect = m_drawables[m_selectedDrawableIndex]->getBoundingBoxOfSlot(slotName.c_str(), slotName.size(), &hit);
	if (hit) return rect;
	return {};
}

bool CDxLibSpinePlayer::getSlotMeshData(const std::string& slotName, SlotMeshData& outData) const
{
	if (m_selectedDrawableIndex < m_drawables.size())
		return m_drawables[m_selectedDrawableIndex]->getSlotMeshData(slotName.c_str(), slotName.size(), outData);
	return false;
}

void CDxLibSpinePlayer::workOutDefaultScale()
{
	m_fDefaultScale  = 1.f;
	m_fDefaultOffset = {};

	const int skelW = static_cast<int>(m_fBaseSize.x);
	const int skelH = static_cast<int>(m_fBaseSize.y);

	int dispW = 0, dispH = 0;
#if defined _WIN32
	DxLib::GetDisplayMaxResolution(&dispW, &dispH);
#elif defined __ANDROID__
	DxLib::GetAndroidDisplayResolution(&dispW, &dispH);
#elif defined __APPLE__
	DxLib::GetDisplayResolution_iOS(&dispW, &dispH);
#endif
	if (dispW == 0 || dispH == 0) return;

	if (skelW > dispW || skelH > dispH)
	{
		const float ratioX = static_cast<float>(dispW) / skelW;
		const float ratioY = static_cast<float>(dispH) / skelH;
		m_fDefaultScale = (ratioX < ratioY) ? ratioX : ratioY;
	}
}

void CDxLibSpinePlayer::workOutDefaultOffset()
{
	if (m_bResetOffsetOnLoad)
	{
		m_fDefaultOffset = { 0.f, 0.f };
		return;
	}

	float minX = FLT_MAX, minY = FLT_MAX;
	for (const auto& d : m_drawables)
	{
		const auto& r = d->getBoundingBox();
		if (r.x < minX) minX = r.x;
		if (r.y < minY) minY = r.y;
	}
	m_fDefaultOffset = { (minX == FLT_MAX) ? 0.f : minX,
	                     (minY == FLT_MAX) ? 0.f : minY };
}
