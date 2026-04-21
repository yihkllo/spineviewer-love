#ifndef DXLIB_SPINE_PLAYER_C_H_
#define DXLIB_SPINE_PLAYER_C_H_

#include "controller.h"
#include "../shared/slot_mesh_data.h"

class CDxLibSpinePlayerC : public CSpinePlayerC
{
public:
	CDxLibSpinePlayerC();
	~CDxLibSpinePlayerC();

	void draw();

	DxLib::MATRIX calculateTransformMatrix() const noexcept;
	DxLib::FLOAT4 getCurrentBoundingOfSlot(const std::string& slotName) const;
	bool getSlotMeshData(const std::string& slotName, SlotMeshData& outData) const;
	void setRenderScreenSize(int width, int height) noexcept;
private:
	void workOutDefaultScale() override;
	void workOutDefaultOffset() override;

	int m_renderScreenWidth = 0;
	int m_renderScreenHeight = 0;
};
#endif
