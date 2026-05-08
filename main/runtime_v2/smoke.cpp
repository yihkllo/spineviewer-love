#include "cpp_runtime_manager.h"

int main()
{
	sl_runtime_v2::CppRuntimeManager manager;
	if (!manager.Resolve("3.8")) return 1;
	if (!manager.Resolve("4.0")) return 2;
	if (!manager.Resolve("4.1")) return 3;
	if (!manager.Resolve("4.2")) return 4;
	return 0;
}
