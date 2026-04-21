#include "spine_runtime_registry.h"

#include <array>
#include <cstring>
#include <memory>
#include <type_traits>

namespace
{
	struct RuntimeDescriptor
	{
		CSpineRuntimeRegistry::ERuntimeSlot slot;
		const char* versionPrefix;
		size_t versionPrefixLength;
		const wchar_t* libraryPath;
		const char* createEntry;
		const char* destroyEntry;
	};

	using RegisterBridgeFunc = void(*)(const DxLibRegerenda*);
	using CreatePlayerFunc = ISpinePlayer * (*)();
	using DestroyPlayerFunc = void(*)(ISpinePlayer*);
	using UniqueModuleHandle = std::unique_ptr<std::remove_pointer<HMODULE>::type, decltype(&::FreeLibrary)>;

	RuntimeDescriptor MakeRuntimeDescriptor(
		CSpineRuntimeRegistry::ERuntimeSlot slot,
		const char* versionPrefix,
		const wchar_t* libraryPath,
		const char* createEntry,
		const char* destroyEntry)
	{
		RuntimeDescriptor descriptor{};
		descriptor.slot = slot;
		descriptor.versionPrefix = versionPrefix;
		descriptor.versionPrefixLength = std::strlen(versionPrefix);
		descriptor.libraryPath = libraryPath;
		descriptor.createEntry = createEntry;
		descriptor.destroyEntry = destroyEntry;
		return descriptor;
	}

	const std::array<RuntimeDescriptor, 9>& RuntimeCatalog()
	{
		static const std::array<RuntimeDescriptor, 9> catalog =
		{
			MakeRuntimeDescriptor(CSpineRuntimeRegistry::ERuntimeSlot::Spine21, "2.1", L"spinedll\\SpineC21.dll", "CreateSpinePlayer21", "DestroySpinePlayer21"),
			MakeRuntimeDescriptor(CSpineRuntimeRegistry::ERuntimeSlot::Spine34, "3.4", L"spinedll\\SpineC34.dll", "CreateSpinePlayer34", "DestroySpinePlayer34"),
			MakeRuntimeDescriptor(CSpineRuntimeRegistry::ERuntimeSlot::Spine35, "3.5", L"spinedll\\SpineC35.dll", "CreateSpinePlayer35", "DestroySpinePlayer35"),
			MakeRuntimeDescriptor(CSpineRuntimeRegistry::ERuntimeSlot::Spine36, "3.6", L"spinedll\\SpineC36.dll", "CreateSpinePlayer36", "DestroySpinePlayer36"),
			MakeRuntimeDescriptor(CSpineRuntimeRegistry::ERuntimeSlot::Spine37, "3.7", L"spinedll\\SpineC37.dll", "CreateSpinePlayer37", "DestroySpinePlayer37"),
			MakeRuntimeDescriptor(CSpineRuntimeRegistry::ERuntimeSlot::Spine38, "3.8", L"spinedll\\SpineCpp38.dll", "CreateSpinePlayer38", "DestroySpinePlayer38"),
			MakeRuntimeDescriptor(CSpineRuntimeRegistry::ERuntimeSlot::Spine40, "4.0", L"spinedll\\SpineCpp40.dll", "CreateSpinePlayer40", "DestroySpinePlayer40"),
			MakeRuntimeDescriptor(CSpineRuntimeRegistry::ERuntimeSlot::Spine41, "4.1", L"spinedll\\SpineCpp41.dll", "CreateSpinePlayer41", "DestroySpinePlayer41"),
			MakeRuntimeDescriptor(CSpineRuntimeRegistry::ERuntimeSlot::Spine42, "4.2", L"spinedll\\SpineCpp42.dll", "CreateSpinePlayer42", "DestroySpinePlayer42")
		};
		return catalog;
	}

	const RuntimeDescriptor* FindRuntimeDescriptor(CSpineRuntimeRegistry::ERuntimeSlot slot)
	{
		const std::array<RuntimeDescriptor, 9>& catalog = RuntimeCatalog();
		for (size_t index = 0; index < catalog.size(); ++index)
		{
			if (catalog[index].slot == slot)
				return &catalog[index];
		}
		return nullptr;
	}

	const RuntimeDescriptor* MatchRuntimeVersion(const char* version)
	{
		if (version == nullptr)
			return nullptr;

		const std::array<RuntimeDescriptor, 9>& catalog = RuntimeCatalog();
		for (size_t index = 0; index < catalog.size(); ++index)
		{
			const RuntimeDescriptor& descriptor = catalog[index];
			if (std::strncmp(version, descriptor.versionPrefix, descriptor.versionPrefixLength) == 0)
				return &descriptor;
		}
		return nullptr;
	}
}

class SpineRuntimeModule
{
public:
	SpineRuntimeModule() = default;
	~SpineRuntimeModule()
	{
		Reset();
	}

	bool Attach(const RuntimeDescriptor& descriptor)
	{
		Reset();

		m_module.reset(::LoadLibraryW(descriptor.libraryPath));
		if (!m_module)
			return false;

		RegisterBridgeFunc registerBridge = reinterpret_cast<RegisterBridgeFunc>(
			::GetProcAddress(m_module.get(), "RegisterDxLibFunctions"));
		if (registerBridge == nullptr)
			return false;
		registerBridge(GetDxLibFunctonsToBeRegistered());

		CreatePlayerFunc createPlayer = reinterpret_cast<CreatePlayerFunc>(
			::GetProcAddress(m_module.get(), descriptor.createEntry));
		m_destroyPlayer = reinterpret_cast<DestroyPlayerFunc>(
			::GetProcAddress(m_module.get(), descriptor.destroyEntry));
		if (createPlayer == nullptr || m_destroyPlayer == nullptr)
			return false;

		m_player = createPlayer();
		return m_player != nullptr;
	}

	ISpinePlayer* Player() const
	{
		return m_player;
	}

private:
	void Reset()
	{
		if (m_player != nullptr && m_destroyPlayer != nullptr)
			m_destroyPlayer(m_player);
		m_player = nullptr;
		m_destroyPlayer = nullptr;
		m_module.reset();
	}

	UniqueModuleHandle m_module{ nullptr, ::FreeLibrary };
	DestroyPlayerFunc m_destroyPlayer = nullptr;
	ISpinePlayer* m_player = nullptr;
};

CSpineRuntimeRegistry::CSpineRuntimeRegistry()
{
	RefreshAvailableRuntimes();
}

CSpineRuntimeRegistry::~CSpineRuntimeRegistry() = default;

bool CSpineRuntimeRegistry::RefreshAvailableRuntimes()
{
	m_allRequestedRuntimesLoaded = true;

	const std::array<RuntimeDescriptor, 9>& catalog = RuntimeCatalog();
	for (size_t descriptorIndex = 0; descriptorIndex < catalog.size(); ++descriptorIndex)
	{
		const RuntimeDescriptor& descriptor = catalog[descriptorIndex];
		const size_t moduleIndex = static_cast<size_t>(descriptor.slot);
		auto module = std::make_unique<SpineRuntimeModule>();
		const bool loaded = module->Attach(descriptor);
		if (!loaded)
			m_allRequestedRuntimesLoaded = false;
		m_runtimeModules[moduleIndex] = std::move(module);
	}

	return m_allRequestedRuntimesLoaded;
}

bool CSpineRuntimeRegistry::AllRequestedRuntimesLoaded() const noexcept
{
	return m_allRequestedRuntimesLoaded;
}

CSpineRuntimeRegistry::ERuntimeSlot CSpineRuntimeRegistry::ResolveVersion(const char* version) const noexcept
{
	const RuntimeDescriptor* descriptor = MatchRuntimeVersion(version);
	return descriptor != nullptr ? descriptor->slot : ERuntimeSlot::Unknown;
}

bool CSpineRuntimeRegistry::SelectRuntime(ERuntimeSlot slot) noexcept
{
	if (slot == ERuntimeSlot::Unknown)
		return false;
	if (FindRuntimeDescriptor(slot) == nullptr)
		return false;

	m_activeSlot = slot;
	return true;
}

CSpineRuntimeRegistry::ERuntimeSlot CSpineRuntimeRegistry::ActiveRuntimeSlot() const noexcept
{
	return m_activeSlot;
}

ISpinePlayer* CSpineRuntimeRegistry::RuntimeAt(ERuntimeSlot slot) const
{
	if (slot == ERuntimeSlot::Unknown)
		return nullptr;

	const size_t index = static_cast<size_t>(slot);
	if (index >= NumOfVersions || !m_runtimeModules[index])
		return nullptr;
	return m_runtimeModules[index]->Player();
}

bool CSpineRuntimeRegistry::HasRuntime(ERuntimeSlot slot) const
{
	return RuntimeAt(slot) != nullptr;
}

ISpinePlayer* CSpineRuntimeRegistry::ActiveRuntime() const
{
	return RuntimeAt(m_activeSlot);
}
