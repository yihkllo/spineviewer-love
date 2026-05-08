#ifndef SPINELOVE_RUNTIME_SHARED_SPINE_SLOT_PROBE_H_
#define SPINELOVE_RUNTIME_SHARED_SPINE_SLOT_PROBE_H_

#include <string>
#include <vector>

#include "../sl_gfx_types.h"
#include "slot_mesh_data.h"

class SlSlotProbe
{
public:
	virtual ~SlSlotProbe() = default;

	virtual const std::vector<std::string>& SlotCatalog() const noexcept = 0;
	virtual SlVec4 MeasureSlotBounds(const std::string& slotName) const = 0;
	virtual bool ReadSlotMesh(const std::string& slotName, ReadSlotMeshData& outData) const = 0;
};

#endif
