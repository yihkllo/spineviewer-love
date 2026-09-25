#ifndef SPINELOVE_RUNTIME_SHARED_SL_SKELETON_PROBE_H_
#define SPINELOVE_RUNTIME_SHARED_SL_SKELETON_PROBE_H_

#include <cstddef>
#include <string>
#include "spinelove/sdk_api.h"

namespace sl_skeleton_probe
{
	enum class FileKind
	{
		Unknown,
		Json,
		Binary,
	};

	struct Result
	{
		FileKind kind = FileKind::Unknown;
		std::string version;

		bool IsSpineSkeleton() const
		{
			return kind != FileKind::Unknown && !version.empty();
		}
	};

	SL_SDK_API Result Inspect(const unsigned char* bytes, size_t byteCount);
}

#endif
