#ifndef SPINELOVE_RUNTIME_V2_CPP_RUNTIME_MANAGER_H_
#define SPINELOVE_RUNTIME_V2_CPP_RUNTIME_MANAGER_H_

#include <array>
#include <memory>

#include "runtime_api.h"

namespace sl_runtime_v2 {

class CppRuntimeManager
{
public:
	CppRuntimeManager();

	IRuntime* Runtime(RuntimeKind kind) const noexcept;
	IRuntime* Resolve(const char* version) const noexcept;

private:
	static constexpr size_t kRuntimeCount = 4;
	std::array<std::unique_ptr<IRuntime>, kRuntimeCount> m_runtimes;
};

}

#endif
