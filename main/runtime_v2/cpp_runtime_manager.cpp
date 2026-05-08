#include "cpp_runtime_manager.h"

#include <cstring>

namespace sl_runtime_v2 {

namespace {

size_t RuntimeIndex(RuntimeKind kind)
{
	switch (kind)
	{
	case RuntimeKind::Cpp38: return 0;
	case RuntimeKind::Cpp40: return 1;
	case RuntimeKind::Cpp41: return 2;
	case RuntimeKind::Cpp42: return 3;
	default: return 0;
	}
}

bool VersionMatches(const char* version, const char* prefix)
{
	if (version == nullptr || prefix == nullptr)
		return false;
	return std::strncmp(version, prefix, std::strlen(prefix)) == 0;
}

}

CppRuntimeManager::CppRuntimeManager()
{
	m_runtimes[RuntimeIndex(RuntimeKind::Cpp38)] = CreateCpp38Runtime();
	m_runtimes[RuntimeIndex(RuntimeKind::Cpp40)] = CreateCpp40Runtime();
	m_runtimes[RuntimeIndex(RuntimeKind::Cpp41)] = CreateCpp41Runtime();
	m_runtimes[RuntimeIndex(RuntimeKind::Cpp42)] = CreateCpp42Runtime();
}

IRuntime* CppRuntimeManager::Runtime(RuntimeKind kind) const noexcept
{
	const size_t index = RuntimeIndex(kind);
	if (index >= m_runtimes.size())
		return nullptr;
	return m_runtimes[index].get();
}

IRuntime* CppRuntimeManager::Resolve(const char* version) const noexcept
{
	for (const auto& runtime : m_runtimes)
	{
		if (runtime && VersionMatches(version, runtime->Info().versionPrefix))
			return runtime.get();
	}
	return nullptr;
}

}
