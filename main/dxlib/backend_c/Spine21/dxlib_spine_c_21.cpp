

#include <float.h>
#include <stdlib.h>
#include <string.h>

#include <spine/extension.h>

#include "dxlib_spine_c_21.h"


void CDxLibSpineDrawableC21::ensureWorldVertices(int count)
{
	if (count > m_worldVerticesCapacity)
	{
		m_worldVertices = static_cast<float*>(realloc(m_worldVertices, sizeof(float) * count));
		m_worldVerticesCapacity = count;
	}
}

void CDxLibSpineDrawableC21::ensureDxLibVertices(int count)
{
	if (count > m_dxLibVerticesCapacity)
	{
		m_dxLibVertices = static_cast<DxLib::VERTEX2D*>(realloc(m_dxLibVertices, sizeof(DxLib::VERTEX2D) * count));
		m_dxLibVerticesCapacity = count;
	}
}

void CDxLibSpineDrawableC21::ensureDxLibIndices(int count)
{
	if (count > m_dxLibIndicesCapacity)
	{
		m_dxLibIndices = static_cast<unsigned short*>(realloc(m_dxLibIndices, sizeof(unsigned short) * count));
		m_dxLibIndicesCapacity = count;
	}
}


#if defined(_WIN32) && defined(_UNICODE)
static wchar_t* WidenPath(const char* path)
{
	int iCharCode  = DxLib::GetUseCharCodeFormat();
	int iWcharCode = DxLib::Get_wchar_t_CharCodeFormat();

	size_t nLen = strlen(path);
	wchar_t* pResult = static_cast<wchar_t*>(malloc((nLen + 1LL) * sizeof(wchar_t)));
	if (pResult == nullptr) return nullptr;
	memset(pResult, 0, (nLen + 1LL) * sizeof(wchar_t));

	int iLen = DxLib::ConvertStringCharCodeFormat(iCharCode, path, iWcharCode, pResult);
	if (iLen == -1)
	{
		free(pResult);
		return nullptr;
	}

	wchar_t* pTemp = static_cast<wchar_t*>(realloc(pResult, iLen));
	if (pTemp != nullptr) pResult = pTemp;

	return pResult;
}
#endif


void _spAtlasPage_createTexture(spAtlasPage* pAtlasPage, const char* path)
{
#if defined(_WIN32) && defined(_UNICODE)
	wchar_t* wcharPath = WidenPath(path);
	if (wcharPath == nullptr) return;
	int iDxLibTexture = DxLib::LoadGraph(wcharPath);
	free(wcharPath);
#else
	int iDxLibTexture = DxLib::LoadGraph(path);
#endif
	if (iDxLibTexture == -1) return;

	pAtlasPage->rendererObject = reinterpret_cast<void*>(static_cast<unsigned long long>(iDxLibTexture));
}

void _spAtlasPage_disposeTexture(spAtlasPage* pAtlasPage)
{
	DxLib::DeleteGraph(static_cast<int>(reinterpret_cast<unsigned long long>(pAtlasPage->rendererObject)));
}

char* _spUtil_readFile(const char* path, int* length)
{
	return _readFile(path, length);
}


CDxLibSpineDrawableC21::CDxLibSpineDrawableC21(spSkeletonData* pSkeletonData, spAnimationStateData* pAnimationStateData)
{
	spBone_setYDown(-1);

	m_skeleton = spSkeleton_create(pSkeletonData);
	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = spAnimationStateData_create(pSkeletonData);
		m_hasOwnAnimationStateData = true;
	}
	m_animationState = spAnimationState_create(pAnimationStateData);
}

CDxLibSpineDrawableC21::~CDxLibSpineDrawableC21()
{
	if (m_animationState != nullptr)
	{
		if (m_hasOwnAnimationStateData)
			spAnimationStateData_dispose(m_animationState->data);
		spAnimationState_dispose(m_animationState);
	}
	if (m_skeleton != nullptr)
		spSkeleton_dispose(m_skeleton);

	free(m_worldVertices);
	free(m_dxLibVertices);
	free(m_dxLibIndices);

	clearLeaveOutList();
}

spSkeleton* CDxLibSpineDrawableC21::skeleton() const noexcept { return m_skeleton; }
spAnimationState* CDxLibSpineDrawableC21::animationState() const noexcept { return m_animationState; }

void CDxLibSpineDrawableC21::premultiplyAlpha(bool premultiplied) noexcept { m_isAlphaPremultiplied = premultiplied; }
bool CDxLibSpineDrawableC21::isAlphaPremultiplied() const noexcept { return m_isAlphaPremultiplied; }
void CDxLibSpineDrawableC21::forceBlendModeNormal(bool toForce) noexcept { m_isToForceBlendModeNormal = toForce; }
bool CDxLibSpineDrawableC21::isBlendModeNormalForced() const noexcept { return m_isToForceBlendModeNormal; }

void CDxLibSpineDrawableC21::update(float fDelta)
{
	if (m_skeleton == nullptr || m_animationState == nullptr) return;

	spSkeleton_update(m_skeleton, fDelta);
	spAnimationState_update(m_animationState, fDelta);
	spAnimationState_apply(m_animationState, m_skeleton);
	spSkeleton_updateWorldTransform(m_skeleton);
}

void CDxLibSpineDrawableC21::draw()
{
	if (m_skeleton == nullptr || m_animationState == nullptr) return;
	if (m_skeleton->a == 0) return;

	static const unsigned short quadIndicesUS[] = { 0, 1, 2, 2, 3, 0 };

	for (int i = 0; i < m_skeleton->slotsCount; ++i)
	{
		spSlot*       pSlot       = m_skeleton->drawOrder[i];
		spAttachment* pAttachment = pSlot->attachment;

		if (pAttachment == nullptr) continue;
		if (isSlotToBeLeftOut(pSlot->data->name)) continue;

		float*          pWorldVerts    = nullptr;
		int             worldVertCount = 0;
		float*          pUVs           = nullptr;
		unsigned short* pIndicesUS     = nullptr;
		int             indicesCount   = 0;
		int*            pIntIndices    = nullptr;

		DxLib::COLOR_F attachmentColor{};
		int iDxLibTexture = -1;

		if (pAttachment->type == SP_ATTACHMENT_REGION)
		{
			spRegionAttachment* pReg = reinterpret_cast<spRegionAttachment*>(pAttachment);
			attachmentColor = { pReg->r, pReg->g, pReg->b, pReg->a };

			ensureWorldVertices(8);
			spRegionAttachment_computeWorldVertices(pReg, pSlot->bone, m_worldVertices);
			pWorldVerts    = m_worldVertices;
			worldVertCount = 8;
			pUVs           = pReg->uvs;
			pIndicesUS     = const_cast<unsigned short*>(quadIndicesUS);
			indicesCount   = 6;

			iDxLibTexture = static_cast<int>(reinterpret_cast<unsigned long long>(
				reinterpret_cast<spAtlasRegion*>(pReg->rendererObject)->page->rendererObject));
		}
		else if (pAttachment->type == SP_ATTACHMENT_MESH)
		{
			spMeshAttachment* pMesh = reinterpret_cast<spMeshAttachment*>(pAttachment);
			attachmentColor = { pMesh->r, pMesh->g, pMesh->b, pMesh->a };

			worldVertCount = pMesh->verticesCount;
			ensureWorldVertices(worldVertCount);
			spMeshAttachment_computeWorldVertices(pMesh, pSlot, m_worldVertices);
			pWorldVerts  = m_worldVertices;
			pUVs         = pMesh->uvs;
			pIntIndices  = pMesh->triangles;
			indicesCount = pMesh->trianglesCount;

			iDxLibTexture = static_cast<int>(reinterpret_cast<unsigned long long>(
				reinterpret_cast<spAtlasRegion*>(pMesh->rendererObject)->page->rendererObject));
		}
		else if (pAttachment->type == SP_ATTACHMENT_SKINNED_MESH)
		{
			spSkinnedMeshAttachment* pMesh = reinterpret_cast<spSkinnedMeshAttachment*>(pAttachment);
			attachmentColor = { pMesh->r, pMesh->g, pMesh->b, pMesh->a };

			worldVertCount = pMesh->uvsCount;
			ensureWorldVertices(worldVertCount);
			spSkinnedMeshAttachment_computeWorldVertices(pMesh, pSlot, m_worldVertices);
			pWorldVerts  = m_worldVertices;
			pUVs         = pMesh->uvs;
			pIntIndices  = pMesh->triangles;
			indicesCount = pMesh->trianglesCount;

			iDxLibTexture = static_cast<int>(reinterpret_cast<unsigned long long>(
				reinterpret_cast<spAtlasRegion*>(pMesh->rendererObject)->page->rendererObject));
		}
		else
		{
			continue;
		}

		if (pWorldVerts == nullptr || indicesCount == 0) continue;

		DxLib::COLOR_F tint{
			m_skeleton->r * pSlot->r * attachmentColor.r,
			m_skeleton->g * pSlot->g * attachmentColor.g,
			m_skeleton->b * pSlot->b * attachmentColor.b,
			m_skeleton->a * pSlot->a * attachmentColor.a
		};

		int vertexCount = worldVertCount / 2;
		ensureDxLibVertices(vertexCount);
		for (int ii = 0, k = 0; ii < worldVertCount; ii += 2, ++k)
		{
			DxLib::VERTEX2D& v = m_dxLibVertices[k];
			v.pos.x = pWorldVerts[ii];
			v.pos.y = pWorldVerts[ii + 1];
			v.pos.z = 0.f;
			v.rhw   = 1.f;
			v.dif.r = static_cast<BYTE>(tint.r * 255.f);
			v.dif.g = static_cast<BYTE>(tint.g * 255.f);
			v.dif.b = static_cast<BYTE>(tint.b * 255.f);
			v.dif.a = static_cast<BYTE>(tint.a * 255.f);
			v.u     = pUVs[ii];
			v.v     = pUVs[ii + 1];
		}

		ensureDxLibIndices(indicesCount);
		if (pIndicesUS)
		{

			memcpy(m_dxLibIndices, pIndicesUS, sizeof(unsigned short) * indicesCount);
		}
		else
		{

			for (int ii = 0; ii < indicesCount; ++ii)
				m_dxLibIndices[ii] = static_cast<unsigned short>(pIntIndices[ii]);
		}

		int iDxLibBlendMode;
		if (!m_isToForceBlendModeNormal && pSlot->data->additiveBlending)
		{
			iDxLibBlendMode = m_isAlphaPremultiplied ? DX_BLENDMODE_PMA_ADD : DX_BLENDMODE_SPINE_ADDITIVE;
		}
		else
		{
			iDxLibBlendMode = m_isAlphaPremultiplied ? DX_BLENDMODE_PMA_ALPHA : DX_BLENDMODE_SPINE_NORMAL;
		}

		DxLib::SetDrawBlendMode(iDxLibBlendMode, 255);
		DxLib::DrawPolygonIndexed2D(
			m_dxLibVertices, vertexCount,
			m_dxLibIndices,  indicesCount / 3,
			iDxLibTexture, TRUE
		);
	}
}

void CDxLibSpineDrawableC21::setLeaveOutList(const char** list, int listCount)
{
	clearLeaveOutList();
	m_leaveOutList = static_cast<char**>(calloc(listCount, sizeof(char*)));
	if (m_leaveOutList == nullptr) return;
	m_leaveOutListCount = listCount;
	for (int i = 0; i < listCount; ++i)
		MALLOC_STR(m_leaveOutList[i], list[i]);
}

DxLib::FLOAT4 CDxLibSpineDrawableC21::getBoundingBox() const
{
	float fMinX = FLT_MAX, fMinY = FLT_MAX;
	float fMaxX = -FLT_MAX, fMaxY = -FLT_MAX;

	int tempCap = 128;
	float* pTemp = static_cast<float*>(malloc(sizeof(float) * tempCap));
	if (pTemp == nullptr) return { 0.f, 0.f, 0.f, 0.f };

	for (int i = 0; i < m_skeleton->slotsCount; ++i)
	{
		spSlot*       pSlot = m_skeleton->drawOrder[i];
		spAttachment* pAtt  = pSlot->attachment;
		if (pAtt == nullptr) continue;

		int cnt = 0;
		if (pAtt->type == SP_ATTACHMENT_REGION)
		{
			cnt = 8;
			if (cnt > tempCap) { tempCap = cnt; pTemp = static_cast<float*>(realloc(pTemp, sizeof(float) * tempCap)); }
			spRegionAttachment_computeWorldVertices(reinterpret_cast<spRegionAttachment*>(pAtt), pSlot->bone, pTemp);
		}
		else if (pAtt->type == SP_ATTACHMENT_MESH)
		{
			spMeshAttachment* pMesh = reinterpret_cast<spMeshAttachment*>(pAtt);
			cnt = pMesh->verticesCount;
			if (cnt > tempCap) { tempCap = cnt; pTemp = static_cast<float*>(realloc(pTemp, sizeof(float) * tempCap)); }
			spMeshAttachment_computeWorldVertices(pMesh, pSlot, pTemp);
		}
		else if (pAtt->type == SP_ATTACHMENT_SKINNED_MESH)
		{
			spSkinnedMeshAttachment* pMesh = reinterpret_cast<spSkinnedMeshAttachment*>(pAtt);
			cnt = pMesh->uvsCount;
			if (cnt > tempCap) { tempCap = cnt; pTemp = static_cast<float*>(realloc(pTemp, sizeof(float) * tempCap)); }
			spSkinnedMeshAttachment_computeWorldVertices(pMesh, pSlot, pTemp);
		}
		else continue;

		for (int ii = 0; ii < cnt; ii += 2)
		{
			float fX = pTemp[ii], fY = pTemp[ii + 1];
			if (fX < fMinX) fMinX = fX;
			if (fY < fMinY) fMinY = fY;
			if (fX > fMaxX) fMaxX = fX;
			if (fY > fMaxY) fMaxY = fY;
		}
	}
	free(pTemp);
	return DxLib::FLOAT4{ fMinX, fMinY, fMaxX - fMinX, fMaxY - fMinY };
}

DxLib::FLOAT4 CDxLibSpineDrawableC21::getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found) const
{
	float fMinX = FLT_MAX, fMinY = FLT_MAX;
	float fMaxX = -FLT_MAX, fMaxY = -FLT_MAX;

	int tempCap = 128;
	float* pTemp = static_cast<float*>(malloc(sizeof(float) * tempCap));
	if (pTemp == nullptr) return { 0.f, 0.f, 0.f, 0.f };

	if (m_skeleton != nullptr)
	{
		for (int i = 0; i < m_skeleton->slotsCount; ++i)
		{
			spSlot*       pSlot = m_skeleton->drawOrder[i];
			spAttachment* pAtt  = pSlot->attachment;
			if (pAtt == nullptr) continue;

			size_t nLen = strlen(pSlot->data->name);
			if (nLen != nameLength) continue;
			if (memcmp(pSlot->data->name, slotName, nLen) != 0) continue;

			int cnt = 0;
			if (pAtt->type == SP_ATTACHMENT_REGION)
			{
				cnt = 8;
				if (cnt > tempCap) { tempCap = cnt; pTemp = static_cast<float*>(realloc(pTemp, sizeof(float) * tempCap)); }
				spRegionAttachment_computeWorldVertices(reinterpret_cast<spRegionAttachment*>(pAtt), pSlot->bone, pTemp);
			}
			else if (pAtt->type == SP_ATTACHMENT_MESH)
			{
				spMeshAttachment* pMesh = reinterpret_cast<spMeshAttachment*>(pAtt);
				cnt = pMesh->verticesCount;
				if (cnt > tempCap) { tempCap = cnt; pTemp = static_cast<float*>(realloc(pTemp, sizeof(float) * tempCap)); }
				spMeshAttachment_computeWorldVertices(pMesh, pSlot, pTemp);
			}
			else if (pAtt->type == SP_ATTACHMENT_SKINNED_MESH)
			{
				spSkinnedMeshAttachment* pMesh = reinterpret_cast<spSkinnedMeshAttachment*>(pAtt);
				cnt = pMesh->uvsCount;
				if (cnt > tempCap) { tempCap = cnt; pTemp = static_cast<float*>(realloc(pTemp, sizeof(float) * tempCap)); }
				spSkinnedMeshAttachment_computeWorldVertices(pMesh, pSlot, pTemp);
			}
			else continue;

			for (int ii = 0; ii < cnt; ii += 2)
			{
				float fX = pTemp[ii], fY = pTemp[ii + 1];
				if (fX < fMinX) fMinX = fX;
				if (fY < fMinY) fMinY = fY;
				if (fX > fMaxX) fMaxX = fX;
				if (fY > fMaxY) fMaxY = fY;
			}
			if (found != nullptr) *found = true;
			break;
		}
	}
	free(pTemp);
	return DxLib::FLOAT4{ fMinX, fMinY, fMaxX - fMinX, fMaxY - fMinY };
}

void CDxLibSpineDrawableC21::clearLeaveOutList()
{
	if (m_leaveOutList != nullptr)
	{
		for (int i = 0; i < m_leaveOutListCount; ++i)
			if (m_leaveOutList[i] != nullptr) FREE(m_leaveOutList[i]);
		FREE(m_leaveOutList);
	}
	m_leaveOutList      = nullptr;
	m_leaveOutListCount = 0;
}

bool CDxLibSpineDrawableC21::isSlotToBeLeftOut(const char* slotName)
{
	if (m_pLeaveOutCallback != nullptr)
		return m_pLeaveOutCallback(slotName, strlen(slotName));

	for (int i = 0; i < m_leaveOutListCount; ++i)
		if (strcmp(slotName, m_leaveOutList[i]) == 0) return true;

	return false;
}

bool CDxLibSpineDrawableC21::getSlotMeshData(const char* slotName, size_t nameLength, SlotMeshData& outData) const
{
	if (m_skeleton == nullptr) return false;

	for (int i = 0; i < m_skeleton->slotsCount; ++i)
	{
		spSlot*       pSlot = m_skeleton->drawOrder[i];
		spAttachment* pAtt  = pSlot->attachment;
		if (pAtt == nullptr) continue;

		size_t nLen = strlen(pSlot->data->name);
		if (nLen != nameLength) continue;
		if (memcmp(pSlot->data->name, slotName, nLen) != 0) continue;

		if (pAtt->type == SP_ATTACHMENT_REGION)
		{
			spRegionAttachment* pRegion = reinterpret_cast<spRegionAttachment*>(pAtt);
			float tmp[8];
			spRegionAttachment_computeWorldVertices(pRegion, pSlot->bone, tmp);
			outData.worldVertices.assign(tmp, tmp + 8);
			outData.triangles = { 0, 1, 2, 2, 3, 0 };
			outData.hullLength = 8;
			outData.isRegion = true;
			outData.uvs.assign(pRegion->uvs, pRegion->uvs + 8);
			outData.textureHandle = static_cast<int>(reinterpret_cast<unsigned long long>(
				reinterpret_cast<spAtlasRegion*>(pRegion->rendererObject)->page->rendererObject));
			return true;
		}
		else if (pAtt->type == SP_ATTACHMENT_MESH)
		{
			spMeshAttachment* pMesh = reinterpret_cast<spMeshAttachment*>(pAtt);
			int vtxLen = pMesh->verticesCount;
			float* pTemp = static_cast<float*>(malloc(sizeof(float) * vtxLen));
			if (pTemp == nullptr) return false;
			spMeshAttachment_computeWorldVertices(pMesh, pSlot, pTemp);
			outData.worldVertices.assign(pTemp, pTemp + vtxLen);
			free(pTemp);
			outData.triangles.resize(pMesh->trianglesCount);
			for (int ti = 0; ti < pMesh->trianglesCount; ++ti)
				outData.triangles[ti] = static_cast<unsigned short>(pMesh->triangles[ti]);
			outData.hullLength = pMesh->hullLength;
			outData.isRegion = false;
			outData.uvs.assign(pMesh->uvs, pMesh->uvs + vtxLen);
			outData.textureHandle = static_cast<int>(reinterpret_cast<unsigned long long>(
				reinterpret_cast<spAtlasRegion*>(pMesh->rendererObject)->page->rendererObject));
			return true;
		}
		else if (pAtt->type == SP_ATTACHMENT_SKINNED_MESH)
		{
			spSkinnedMeshAttachment* pMesh = reinterpret_cast<spSkinnedMeshAttachment*>(pAtt);
			int vtxLen = pMesh->uvsCount;
			float* pTemp = static_cast<float*>(malloc(sizeof(float) * vtxLen));
			if (pTemp == nullptr) return false;
			spSkinnedMeshAttachment_computeWorldVertices(pMesh, pSlot, pTemp);
			outData.worldVertices.assign(pTemp, pTemp + vtxLen);
			free(pTemp);
			outData.triangles.resize(pMesh->trianglesCount);
			for (int ti = 0; ti < pMesh->trianglesCount; ++ti)
				outData.triangles[ti] = static_cast<unsigned short>(pMesh->triangles[ti]);
			outData.hullLength = pMesh->hullLength;
			outData.isRegion = false;
			outData.uvs.assign(pMesh->uvs, pMesh->uvs + vtxLen);
			outData.textureHandle = static_cast<int>(reinterpret_cast<unsigned long long>(
				reinterpret_cast<spAtlasRegion*>(pMesh->rendererObject)->page->rendererObject));
			return true;
		}
		return false;
	}
	return false;
}
