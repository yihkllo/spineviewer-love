#ifndef SPINELOVE_SPINE_RUNTIME_REGISTRY_H_
#define SPINELOVE_SPINE_RUNTIME_REGISTRY_H_

#include <array>
#include <memory>

#include "dxlib/shared/dll_bridge.h"
#include "dxlib/shared/dll_entry.h"

class SpineRuntimeModule;

class CSpineRuntimeRegistry
{
public:
	CSpineRuntimeRegistry();
	~CSpineRuntimeRegistry();

	bool RefreshAvailableRuntimes();
	bool AllRequestedRuntimesLoaded() const noexcept;

	enum class ERuntimeSlot : uint8_t
	{
		Unknown = static_cast<uint8_t>(-1U),
		Spine21 = 0,
		Spine34,
		Spine35,
		Spine36,
		Spine37,
		Spine38,
		Spine40,
		Spine41,
		Spine42,
		kMax
	};

	ERuntimeSlot ResolveVersion(const char* version) const noexcept;
	bool SelectRuntime(ERuntimeSlot slot) noexcept;
	ERuntimeSlot ActiveRuntimeSlot() const noexcept;

	ISpinePlayer* RuntimeAt(ERuntimeSlot slot) const;
	bool HasRuntime(ERuntimeSlot slot) const;
	ISpinePlayer* ActiveRuntime() const;

private:
	static constexpr size_t NumOfVersions = 9;
	static_assert(NumOfVersions == static_cast<uint8_t>(ERuntimeSlot::kMax), "Spine version count does not match enumeration.");

	std::array<std::unique_ptr<SpineRuntimeModule>, NumOfVersions> m_runtimeModules;
	ERuntimeSlot m_activeSlot = ERuntimeSlot::Spine38;
	bool m_allRequestedRuntimesLoaded = true;
};

#endif

