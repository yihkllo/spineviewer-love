

#include "sl_gfx_pixelmap.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>


CDxLibMap::CDxLibMap(int iTextureHandle)
	:m_imageHandle(iTextureHandle)
{
	ReadPixels();
}

CDxLibMap::~CDxLibMap()
{
	Unlock();
}

bool CDxLibMap::IsAccessible() const
{
	return m_isLocked;
}

bool CDxLibMap::ReadPixels()
{
	int iRet = DxLib::GetGraphSize(m_imageHandle, &width, &height);
	if (iRet == -1)return false;

	void* pData = nullptr;
	DxLib::COLORDATA* pFormat = nullptr;
	iRet = DxLib::GraphLock(m_imageHandle, &stride, &pData, &pFormat);
	if (iRet != -1)
	{
		m_isLocked = true;
		pPixels = static_cast<unsigned char*>(pData);
		pColorData = pFormat;
	}

	return m_isLocked;
}

void CDxLibMap::Unlock() const
{
	if (m_imageHandle != -1 && m_isLocked)
	{
		DxLib::GraphUnLock(m_imageHandle);
	}
}
