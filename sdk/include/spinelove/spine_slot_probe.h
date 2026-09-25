#ifndef SPINELOVE_RUNTIME_SHARED_SPINE_SLOT_PROBE_H_
#define SPINELOVE_RUNTIME_SHARED_SPINE_SLOT_PROBE_H_

#include <string>
#include <vector>

#include "spinelove/sl_gfx_types.h"
#include "spinelove/slot_mesh_data.h"

enum class SlSlotAttachmentMode
{
	Preserve,
	SetupIfEmpty,
	NamedIfEmpty,
	Clear,
};

class SlSlotProbe
{
public:
	virtual ~SlSlotProbe() = default;

	virtual const std::vector<std::string>& SlotCatalog() const noexcept = 0;
	virtual SlVec4 MeasureSlotBounds(const std::string& slotName) const = 0;
	virtual bool ReadSlotMesh(const std::string& slotName, ReadSlotMeshData& outData) const = 0;
	virtual bool SetSlotOverride(const std::string&, float, SlSlotAttachmentMode) { return false; }
	virtual void ClearSlotOverrides() {}
	virtual bool ReadBoneTransform(const std::string&, SlMatrix4&) const { return false; }
};

#endif
