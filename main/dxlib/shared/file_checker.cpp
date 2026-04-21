

#include "file_checker.h"

#if defined (ALLOW_CPP20_STL)
	#include <algorithm>
	#include <cctype>
#else
	#include <ctype.h>
	#include <memory.h>
	#include <stdint.h>
#endif

namespace spine_file_verifier
{


#ifdef ALLOW_CPP20_STL

	static bool IsHashString(std::string_view s)
	{
		return std::ranges::all_of(s, [](const char& c) {return std::isalnum(static_cast<const unsigned char>(c)) != 0 || c == '+' || c == '/'; });
	}


	static bool IsVersionString(std::string_view s)
	{
		return std::ranges::all_of(s, [](const char& c) {return std::isdigit(static_cast<const unsigned char>(c)) != 0 || c == '.'; });
	}
#else

	static const unsigned char* MemSearch(const unsigned char* first1, const unsigned char* end1, const unsigned char* first2, const unsigned char* end2)
	{
		const size_t length = end2 - first2;
		const unsigned char* end = end1 - length;
		for (const unsigned char* pos = first1; pos < end; ++pos)
		{
			if (::memcmp(pos, first2, length) == 0)
			{
				return pos;
			}
		}

		return end1;
	}

	static bool IsHashString(const unsigned char* s, size_t length)
	{
		for (size_t i = 0; i < length; ++i)
		{
			const auto& c = s[i];
			if (!(::isalnum(c) != 0 || c == '+' || c == '/'))
			{
				return false;
			}
		}

		return true;
	}

	static bool IsVersionString(const unsigned char* s, size_t length)
	{
		for (size_t i = 0; i < length; ++i)
		{
			const auto& c = s[i];
			if (!(::isdigit(c) != 0 || c == '.'))
			{
				return false;
			}
		}

		return true;
	}
#endif


	static bool IsLikelyJsonSkeleton(const unsigned char* pData, size_t dataSize, SkeletonMetadata& skeletonMetaData)
	{


		static constexpr size_t MinJsonSkeletonFileSize = 160;
		if (dataSize < MinJsonSkeletonFileSize)return false;

		static constexpr const unsigned char skeleton[] = R"("skeleton")";
		static constexpr const unsigned char versionKey[] = R"("spine")";

		const unsigned char* begin = pData;

		const size_t searchSize = dataSize < 512 ? dataSize : 512;
		const unsigned char* end = begin + searchSize;
#ifdef ALLOW_CPP20_STL
		const unsigned char* iter = std::search(begin, end, skeleton, skeleton + sizeof(skeleton) - 1);
#else
		const unsigned char* iter = MemSearch(begin, end, skeleton, skeleton + sizeof(skeleton) - 1);
#endif
		if (iter == end)return false;
#ifdef ALLOW_CPP20_STL
		iter = std::search(begin, end, versionKey, versionKey + sizeof(versionKey) - 1);
#else
		iter = MemSearch(begin, end, versionKey, versionKey + sizeof(versionKey) - 1);
#endif
		if (iter == end)return false;

		iter += sizeof(versionKey) - 1;
		const unsigned char* versionStart = static_cast<const unsigned char*>(::memchr(iter, '"', end - iter));
		if (versionStart == nullptr)return false;
		++versionStart;
		const unsigned char* versionEnd = static_cast<const unsigned char*>(::memchr(versionStart, '"', end - versionStart));
		if (versionEnd == nullptr)return false;

#ifdef ALLOW_CPP20_STL
		skeletonMetaData.version = std::string_view(reinterpret_cast<const std::string_view::value_type*>(versionStart), versionEnd - versionStart);

		return IsVersionString(skeletonMetaData.version);
#else
		skeletonMetaData.version = versionStart;
		skeletonMetaData.versionLength = versionEnd - versionStart;

		return IsVersionString(skeletonMetaData.version, skeletonMetaData.versionLength);
#endif
	}


	static bool StartsWithHexHashPrecedingVersion(const unsigned char* pData, size_t dataSize, SkeletonMetadata& skeletonMetaData)
	{

		static constexpr const size_t HashLength = 8;
		static constexpr const size_t VersionEndPos = HashLength + 7;
		if (dataSize < VersionEndPos)return false;

		size_t pos = HashLength;
		uint8_t versionLength = pData[HashLength];
		if (versionLength < 1)return false;
		pos += sizeof(uint8_t);
		--versionLength;
		if (dataSize < pos + versionLength)return false;

#ifdef ALLOW_CPP20_STL
		skeletonMetaData.version = std::string_view(reinterpret_cast<const std::string_view::value_type*>(&pData[pos]), versionLength);

		return IsVersionString(skeletonMetaData.version);
#else
		skeletonMetaData.version = &pData[pos];
		skeletonMetaData.versionLength = versionLength;

		return IsVersionString(skeletonMetaData.version, skeletonMetaData.versionLength);
#endif
	}


	static bool StartsWithStringHashPrecedingVersion(const unsigned char* pData, size_t dataSize, SkeletonMetadata& skeletonMetaData)
	{


		static constexpr const size_t MinBinarySkeletonFileSize = 40;
		if (dataSize < MinBinarySkeletonFileSize) return false;

		size_t pos = 0;
		{
			uint8_t hashLength = pData[pos];
			if (hashLength < 1)return false;
			pos += sizeof(uint8_t);
			--hashLength;
			if (dataSize < pos + hashLength)return false;

#ifdef ALLOW_CPP20_STL
			std::string_view hash(reinterpret_cast<const std::string_view::value_type*>(&pData[pos]), hashLength);

			if (!IsHashString(hash)) return false;
#else
			if (!IsHashString(&pData[pos], hashLength))return false;
#endif

			pos += hashLength;
		}

		uint8_t versionLength = pData[pos];
		pos += sizeof(uint8_t);
		--versionLength;
		if (dataSize < pos + versionLength)return false;

#ifdef ALLOW_CPP20_STL
		skeletonMetaData.version = std::string_view(reinterpret_cast<const std::string_view::value_type*>(&pData[pos]), versionLength);

		return IsVersionString(skeletonMetaData.version);
#else
		skeletonMetaData.version = &pData[pos];
		skeletonMetaData.versionLength = versionLength;

		return IsVersionString(skeletonMetaData.version, skeletonMetaData.versionLength);
#endif
	}


}

spine_file_verifier::SkeletonMetadata spine_file_verifier::VerifySkeletonFileData(const unsigned char* pFileData, size_t dataLength)
{
	SkeletonMetadata skeletonMetaData;

	if (IsLikelyJsonSkeleton(pFileData, dataLength, skeletonMetaData))
	{
		skeletonMetaData.skeletonFormat = SkeletonFormat::Json;
	}
	else if (StartsWithHexHashPrecedingVersion(pFileData, dataLength, skeletonMetaData))
	{
		skeletonMetaData.skeletonFormat = SkeletonFormat::Binary;
	}
	else if (StartsWithStringHashPrecedingVersion(pFileData, dataLength, skeletonMetaData))
	{
		skeletonMetaData.skeletonFormat = SkeletonFormat::Binary;
	}

    return skeletonMetaData;
}
