#ifndef DXLIB_SPINE_H_
#define DXLIB_SPINE_H_


#undef min
#undef max
#include <spine/spine.h>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

#if defined(SPINE_RUNTIME_DLL_BUILD)
#include "../shared/dll_bridge.h"
#endif

#include "../shared/slot_mesh_data.h"

class CDxLibSpineDrawable
{
public:
	CDxLibSpineDrawable(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData = nullptr);
	~CDxLibSpineDrawable();

	spine::Skeleton* skeleton() const noexcept;
	spine::AnimationState* animationState() const noexcept;

	void premultiplyAlpha(bool premultiplied) noexcept;
	bool isAlphaPremultiplied() const noexcept;

	void forceBlendModeNormal(bool toForce) noexcept;
	bool isBlendModeNormalForced() const noexcept;

	void update(float fDelta);
	void draw();


	void setLeaveOutList(spine::Vector<spine::String> &list);
	void setLeaveOutCallback(bool (*pFunc)(const char*, size_t)) { m_pLeaveOutCallback = pFunc; }

	DxLib::FLOAT4 getBoundingBox() const;
	DxLib::FLOAT4 getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found = nullptr) const;
	bool getSlotMeshData(const char* slotName, size_t nameLength, SlotMeshData& outData) const;
private:
	bool m_hasOwnAnimationStateData = false;
	bool m_isAlphaPremultiplied = true;
	bool m_isToForceBlendModeNormal = false;

	spine::Skeleton* m_skeleton = nullptr;
	spine::AnimationState* m_animationState = nullptr;

	spine::Vector<float> m_worldVertices;
	spine::Vector<DxLib::VERTEX2D> m_dxLibVertices;

	spine::Vector<unsigned short> m_quadIndices;

	spine::SkeletonClipping m_clipper;

	spine::Vector<spine::String> m_leaveOutList;
	bool (*m_pLeaveOutCallback)(const char*, size_t) = nullptr;

	bool IsToBeLeftOut(const spine::String& slotName);
};

class CDxLibTextureLoader : public spine::TextureLoader
{
public:
	void load(spine::AtlasPage& page, const spine::String& path) override;
	void unload(void* texture) override;

	
	
	void setPremultiplyOnLoad(bool premul) noexcept { m_premultiplyOnLoad = premul; }
	bool getPremultiplyOnLoad() const noexcept { return m_premultiplyOnLoad; }

private:
	
	
	bool m_premultiplyOnLoad = false;
};

#endif
