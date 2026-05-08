#ifndef SPINELOVE_RUNTIME_SHARED_SL_SKELETON_PROBE_H_
#define SPINELOVE_RUNTIME_SHARED_SL_SKELETON_PROBE_H_

#include <cstddef>
#include <string>

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

	Result Inspect(const unsigned char* bytes, size_t byteCount);
}

#endif
