

#include <cmath>
#include "player.h"

CDxLibSpinePlayerC::CDxLibSpinePlayerC()
{

}

CDxLibSpinePlayerC::~CDxLibSpinePlayerC()
{

}

void CDxLibSpinePlayerC::draw()
{
	if (!m_drawables.empty())
	{
		DxLib::MATRIX matrix = calculateTransformMatrix();
		DxLib::SetTransformTo2D(&matrix);

		if (!m_isDrawOrderReversed)
		{
			for (size_t i = 0; i < m_drawables.size(); ++i)
			{
				m_drawables[i]->draw();
			}
		}
		else
		{
			for (long long i = m_drawables.size() - 1; i >= 0; --i)
			{
				m_drawables[i]->draw();
			}
		}

		DxLib::ResetTransformTo2D();
	}
}

DxLib::MATRIX CDxLibSpinePlayerC::calculateTransformMatrix() const noexcept
{
	int iScreenWidth = 0;
	int iScreenHeight = 0;
	if (m_renderScreenWidth > 0 && m_renderScreenHeight > 0)
	{
		iScreenWidth = m_renderScreenWidth;
		iScreenHeight = m_renderScreenHeight;
	}
	else
	{
		DxLib::GetDrawScreenSize(&iScreenWidth, &iScreenHeight);
	}

	float fCenterX = m_fBaseSize.x / 2.f;
	float fCenterY = m_fBaseSize.y / 2.f;

	DxLib::MATRIX toOrigin = DxLib::MGetTranslate(DxLib::VGet(-fCenterX, -fCenterY, 0.f));

	float fScaleX = m_bFlipX ? -m_fSkeletonScale : m_fSkeletonScale;
	DxLib::MATRIX scaleMatrix = DxLib::MGetScale(DxLib::VGet(fScaleX, m_fSkeletonScale, 1.f));

	static constexpr float kHalfPi = 1.5707963267948966f;
	DxLib::MATRIX rotMatrix = DxLib::MGetRotZ(kHalfPi * m_iRotationSteps);

	DxLib::MATRIX toScreen = DxLib::MGetTranslate(DxLib::VGet(iScreenWidth / 2.f, iScreenHeight / 2.f, 0.f));

	DxLib::MATRIX m = DxLib::MMult(toOrigin, scaleMatrix);
	m = DxLib::MMult(m, rotMatrix);
	m = DxLib::MMult(m, toScreen);

	return m;
}

DxLib::FLOAT4 CDxLibSpinePlayerC::getCurrentBoundingOfSlot(const std::string& slotName) const
{
	bool found = false;
	for (const auto& drawable : m_drawables)
	{
		const auto rect = drawable->getBoundingBoxOfSlot(slotName.c_str(), slotName.size(), &found);
		if (found)
		{
			return rect;
		}
	}
	return {};
}

bool CDxLibSpinePlayerC::getSlotMeshData(const std::string& slotName, SlotMeshData& outData) const
{
	for (const auto& drawable : m_drawables)
	{
		if (drawable->getSlotMeshData(slotName.c_str(), slotName.size(), outData))
			return true;
	}
	return false;
}

void CDxLibSpinePlayerC::workOutDefaultScale()
{
	m_fDefaultScale = 1.f;

	int iSkeletonWidth = static_cast<int>(m_fBaseSize.x);
	int iSkeletonHeight = static_cast<int>(m_fBaseSize.y);

	int iDisplayWidth = 0;
	int iDisplayHeight = 0;
#if defined _WIN32
	DxLib::GetDisplayMaxResolution(&iDisplayWidth, &iDisplayHeight);
#elif defined __ANDROID__
	DxLib::GetAndroidDisplayResolution(&iDisplayWidth, &iDisplayHeight);
#elif defined __APPLE__
	DxLib::GetDisplayResolution_iOS(&iDisplayWidth, &iDisplayHeight);
#endif
	if (iDisplayWidth == 0 || iDisplayHeight == 0)return;

	if (iSkeletonWidth > iDisplayWidth || iSkeletonHeight > iDisplayHeight)
	{
		float fScaleX = static_cast<float>(iDisplayWidth) / iSkeletonWidth;
		float fScaleY = static_cast<float>(iDisplayHeight) / iSkeletonHeight;

		if (fScaleX > fScaleY)
		{
			m_fDefaultScale = fScaleY;
		}
		else
		{
			m_fDefaultScale = fScaleX;
		}
	}
}

void CDxLibSpinePlayerC::workOutDefaultOffset()
{
	if (m_bResetOffsetOnLoad)
	{
		m_fDefaultOffset = { 0.f, 0.f };
		return;
	}

	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;

	for (const auto& pDrawable : m_drawables)
	{
		const auto rect = pDrawable->getBoundingBox();
		fMinX = (std::min)(fMinX, rect.x);
		fMinY = (std::min)(fMinY, rect.y);
	}

	m_fDefaultOffset = { fMinX == FLT_MAX ? 0 : fMinX, fMinY == FLT_MAX ? 0 : fMinY };
}

void CDxLibSpinePlayerC::setRenderScreenSize(int width, int height) noexcept
{
	m_renderScreenWidth = width;
	m_renderScreenHeight = height;
}
