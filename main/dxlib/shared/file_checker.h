#ifndef SPINE_FILE_VERIFIER_H_
#define SPINE_FILE_VERIFIER_H_

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L) || (defined(__cplusplus) && __cplusplus >= 202002L)
	#define ALLOW_CPP20_STL
	#include <string_view>
#endif

namespace spine_file_verifier
{
	enum class SkeletonFormat
	{
		Neither,
		Json,
		Binary,
	};


	struct SkeletonMetadata
	{
		SkeletonFormat skeletonFormat = SkeletonFormat::Neither;
#if defined (ALLOW_CPP20_STL)
		std::string_view version;
#else
		const unsigned char* version = nullptr;
		size_t versionLength = 0;
#endif
	};

	SkeletonMetadata VerifySkeletonFileData(const unsigned char* pFileData, size_t dataLength);
}

#endif
