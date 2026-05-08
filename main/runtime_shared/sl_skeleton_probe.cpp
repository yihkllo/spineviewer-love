#include "sl_skeleton_probe.h"

#include <cctype>
#include <cstdint>

namespace sl_skeleton_probe
{
namespace
{
	bool IsJsonSpace(unsigned char c)
	{
		return c == ' ' || c == '\r' || c == '\n' || c == '\t';
	}

	void SkipSpaces(const unsigned char* bytes, size_t byteCount, size_t& pos)
	{
		while (pos < byteCount && IsJsonSpace(bytes[pos]))
			++pos;
	}

	bool IsHexDigit(unsigned char c)
	{
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
	}

	bool ReadJsonQuotedText(const unsigned char* bytes, size_t byteCount, size_t& pos, std::string& out)
	{
		out.clear();
		if (pos >= byteCount || bytes[pos] != '"')
			return false;

		++pos;
		while (pos < byteCount)
		{
			const unsigned char c = bytes[pos++];
			if (c == '"')
				return true;

			if (c != '\\')
			{
				out.push_back(static_cast<char>(c));
				continue;
			}

			if (pos >= byteCount)
				return false;

			const unsigned char escaped = bytes[pos++];
			if (escaped == 'u')
			{
				for (int i = 0; i < 4; ++i)
				{
					if (pos >= byteCount || !IsHexDigit(bytes[pos]))
						return false;
					++pos;
				}
				out.push_back('?');
			}
			else
			{
				out.push_back(static_cast<char>(escaped));
			}
		}

		return false;
	}

	bool FindJsonObjectEnd(const unsigned char* bytes, size_t byteCount, size_t objectStart, size_t& objectEnd)
	{
		if (objectStart >= byteCount || bytes[objectStart] != '{')
			return false;

		size_t pos = objectStart;
		int depth = 0;
		std::string ignored;
		while (pos < byteCount)
		{
			const unsigned char c = bytes[pos];
			if (c == '"')
			{
				if (!ReadJsonQuotedText(bytes, byteCount, pos, ignored))
					return false;
				continue;
			}

			if (c == '{')
			{
				++depth;
			}
			else if (c == '}')
			{
				--depth;
				if (depth == 0)
				{
					objectEnd = pos + 1;
					return true;
				}
				if (depth < 0)
					return false;
			}

			++pos;
		}

		return false;
	}

	bool ReadJsonStringMember(
		const unsigned char* bytes,
		size_t begin,
		size_t end,
		const char* wantedKey,
		std::string& value)
	{
		size_t pos = begin;
		std::string key;
		while (pos < end)
		{
			if (bytes[pos] != '"')
			{
				++pos;
				continue;
			}

			if (!ReadJsonQuotedText(bytes, end, pos, key))
				return false;

			SkipSpaces(bytes, end, pos);
			if (pos >= end || bytes[pos] != ':')
				continue;

			++pos;
			SkipSpaces(bytes, end, pos);
			if (key != wantedKey)
				continue;

			return ReadJsonQuotedText(bytes, end, pos, value);
		}

		return false;
	}

	bool ReadSkeletonObjectRange(const unsigned char* bytes, size_t byteCount, size_t& objectBegin, size_t& objectEnd)
	{
		size_t pos = 0;
		std::string key;
		while (pos < byteCount)
		{
			if (bytes[pos] != '"')
			{
				++pos;
				continue;
			}

			if (!ReadJsonQuotedText(bytes, byteCount, pos, key))
				return false;

			SkipSpaces(bytes, byteCount, pos);
			if (pos >= byteCount || bytes[pos] != ':')
				continue;

			++pos;
			SkipSpaces(bytes, byteCount, pos);
			if (key == "skeleton" && pos < byteCount && bytes[pos] == '{')
			{
				objectBegin = pos;
				return FindJsonObjectEnd(bytes, byteCount, objectBegin, objectEnd);
			}
		}

		return false;
	}

	bool LooksLikeVersion(const std::string& text)
	{
		if (text.empty() || text.size() > 31)
			return false;

		bool hasDot = false;
		for (size_t i = 0; i < text.size(); ++i)
		{
			const unsigned char c = static_cast<unsigned char>(text[i]);
			if (c == '.')
			{
				hasDot = true;
				continue;
			}
			if (std::isdigit(c) == 0)
				return false;
		}

		return hasDot && std::isdigit(static_cast<unsigned char>(text[0])) != 0;
	}

	bool TryJsonProbe(const unsigned char* bytes, size_t byteCount, Result& result)
	{
		size_t pos = 0;
		SkipSpaces(bytes, byteCount, pos);
		if (pos >= byteCount || bytes[pos] != '{')
			return false;

		size_t skeletonBegin = 0;
		size_t skeletonEnd = 0;
		std::string version;
		if (!ReadSkeletonObjectRange(bytes, byteCount, skeletonBegin, skeletonEnd))
			return false;

		if (!ReadJsonStringMember(bytes, skeletonBegin, skeletonEnd, "spine", version))
			return false;

		if (!LooksLikeVersion(version))
			return false;

		result.kind = FileKind::Json;
		result.version = version;
		return true;
	}

	bool ReadVarintLength(const unsigned char* bytes, size_t byteCount, size_t& pos, uint32_t& value)
	{
		value = 0;
		for (int shift = 0; shift <= 28; shift += 7)
		{
			if (pos >= byteCount)
				return false;

			const uint32_t b = bytes[pos++];
			value |= (b & 0x7Fu) << shift;
			if ((b & 0x80u) == 0)
				return true;
		}

		return false;
	}

	bool ReadSpineBinaryString(const unsigned char* bytes, size_t byteCount, size_t& pos, std::string& text)
	{
		uint32_t encodedLength = 0;
		if (!ReadVarintLength(bytes, byteCount, pos, encodedLength))
			return false;

		if (encodedLength == 0)
			return false;

		const uint32_t byteLength = encodedLength - 1;
		if (byteLength > 512 || pos + byteLength > byteCount)
			return false;

		text.assign(reinterpret_cast<const char*>(bytes + pos), reinterpret_cast<const char*>(bytes + pos + byteLength));
		pos += byteLength;
		return true;
	}

	bool TryBinaryProbeAfterFixedHash(const unsigned char* bytes, size_t byteCount, Result& result)
	{
		if (byteCount < 10)
			return false;

		size_t pos = 8;
		std::string version;
		if (!ReadSpineBinaryString(bytes, byteCount, pos, version))
			return false;

		if (!LooksLikeVersion(version))
			return false;

		result.kind = FileKind::Binary;
		result.version = version;
		return true;
	}

	bool TryBinaryProbeAfterStringHash(const unsigned char* bytes, size_t byteCount, Result& result)
	{
		size_t pos = 0;
		std::string hash;
		if (!ReadSpineBinaryString(bytes, byteCount, pos, hash))
			return false;

		std::string version;
		if (!ReadSpineBinaryString(bytes, byteCount, pos, version))
			return false;

		if (!LooksLikeVersion(version))
			return false;

		result.kind = FileKind::Binary;
		result.version = version;
		return true;
	}
}

Result Inspect(const unsigned char* bytes, size_t byteCount)
{
	Result result;
	if (bytes == nullptr || byteCount == 0)
		return result;

	if (TryJsonProbe(bytes, byteCount, result))
		return result;

	if (TryBinaryProbeAfterFixedHash(bytes, byteCount, result))
		return result;

	TryBinaryProbeAfterStringHash(bytes, byteCount, result);
	return result;
}
}
