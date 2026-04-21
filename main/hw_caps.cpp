
#include <intrin.h>

#include "hw_caps.h"


bool cpu::IsAmd()
{
	int cpuInfo[4]{};
	__cpuid(cpuInfo, 0x00);

	return cpuInfo[1] == 0x68747541 && cpuInfo[2] == 0x444D4163 && cpuInfo[3] == 0x69746E65;
}

bool cpu::IsIntel()
{
	int cpuInfo[4]{};
	__cpuid(cpuInfo, 0x00);

	return cpuInfo[1] == 0x756e6547 && cpuInfo[2] == 0x6c65746e && cpuInfo[3] == 0x49656e69;
}
