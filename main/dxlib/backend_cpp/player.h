#ifndef DXLIB_SPINE_PLAYER_H_
#define DXLIB_SPINE_PLAYER_H_

#include "controller.h"
#include "../shared/slot_mesh_data.h"


class CDxLibSpinePlayer : public CSpinePlayer
{
public:
	CDxLibSpinePlayer();
	~CDxLibSpinePlayer();


	void draw();
	DxLib::MATRIX calculateTransformMatrix() const noexcept;


	void setRenderScreenSize(int width, int height) noexcept;


	DxLib::FLOAT4 getCurrentBoundingOfSlot(const std::string& slotName) const;
	bool getSlotMeshData(const std::string& slotName, SlotMeshData& outData) const;

private:
	void workOutDefaultScale() override;
	void workOutDefaultOffset() override;


	int m_renderWidth  = 0;
	int m_renderHeight = 0;
};

#endif
