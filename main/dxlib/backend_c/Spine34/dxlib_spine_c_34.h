#ifndef DXLIB_SPINE_C_34_H_
#define DXLIB_SPINE_C_34_H_

#include <spine/spine.h>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

#if defined(SPINE_RUNTIME_DLL_BUILD)
#include "../../shared/dll_bridge.h"
#endif

#include "../../shared/slot_mesh_data.h"

class CDxLibSpineDrawableC34
{
public:
	CDxLibSpineDrawableC34(spSkeletonData* pSkeletonData, spAnimationStateData* pAnimationStateData = nullptr);
	~CDxLibSpineDrawableC34();

	spSkeleton* skeleton() const noexcept;
	spAnimationState* animationState() const noexcept;

	void premultiplyAlpha(bool premultiplied) noexcept;
	bool isAlphaPremultiplied() const noexcept;

	void forceBlendModeNormal(bool toForce) noexcept;
	bool isBlendModeNormalForced() const noexcept;

	void update(float fDelta);
	void draw();

	void setLeaveOutList(const char** list, int listCount);
	void setLeaveOutCallback(bool (*pFunc)(const char*, size_t)) { m_pLeaveOutCallback = pFunc; }

	DxLib::FLOAT4 getBoundingBox() const;
	DxLib::FLOAT4 getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found = nullptr) const;
	bool getSlotMeshData(const char* slotName, size_t nameLength, SlotMeshData& outData) const;

private:
	bool m_hasOwnAnimationStateData = false;
	bool m_isAlphaPremultiplied = true;
	bool m_isToForceBlendModeNormal = false;

	spSkeleton* m_skeleton = nullptr;
	spAnimationState* m_animationState = nullptr;


	float*            m_worldVertices = nullptr;
	int               m_worldVerticesCapacity = 0;

	DxLib::VERTEX2D*  m_dxLibVertices = nullptr;
	int               m_dxLibVerticesCapacity = 0;

	unsigned short*   m_dxLibIndices = nullptr;
	int               m_dxLibIndicesCapacity = 0;

	char** m_leaveOutList = nullptr;
	int    m_leaveOutListCount = 0;
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;

	void ensureWorldVertices(int count);
	void ensureDxLibVertices(int count);
	void ensureDxLibIndices(int count);

	void clearLeaveOutList();
	bool isSlotToBeLeftOut(const char* slotName);
};

#endif
