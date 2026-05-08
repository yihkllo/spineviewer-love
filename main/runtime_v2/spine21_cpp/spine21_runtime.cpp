#include "runtime_api.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sl_runtime_v2 {
namespace {

bool FindMatchingBracket(const std::string& text, std::size_t openPos, char openChar, char closeChar,
                         std::size_t& closePos);

std::string Trim(std::string text)
{
    std::size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin])))
        ++begin;

    std::size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])))
        --end;

    return text.substr(begin, end - begin);
}

bool ReadTextFile(const std::string& path, std::string& outText)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return false;

    outText.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    return true;
}

std::string ParentPathOf(const std::string& path)
{
    const std::size_t slash = path.find_last_of("\\/");
    if (slash == std::string::npos)
        return std::string();
    return path.substr(0, slash);
}

bool IsAbsolutePath(const std::string& path)
{
    if (path.size() >= 2 && std::isalpha(static_cast<unsigned char>(path[0])) && path[1] == ':')
        return true;
    return !path.empty() && (path[0] == '/' || path[0] == '\\');
}

std::string JoinPath(const std::string& base, const std::string& leaf)
{
    if (leaf.empty())
        return leaf;

    if (IsAbsolutePath(leaf) || base.empty())
        return leaf;

    const char last = base.back();
    if (last == '/' || last == '\\')
        return base + leaf;
    return base + "\\" + leaf;
}

bool SkipWhitespace(const std::string& text, std::size_t& pos)
{
    while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos])))
        ++pos;
    return pos < text.size();
}

bool ReadJsonString(const std::string& text, std::size_t& pos, std::string& out)
{
    if (pos >= text.size() || text[pos] != '"')
        return false;

    ++pos;
    out.clear();
    while (pos < text.size())
    {
        const char c = text[pos++];
        if (c == '"')
            return true;

        if (c != '\\')
        {
            out.push_back(c);
            continue;
        }

        if (pos >= text.size())
            return false;

        const char esc = text[pos++];
        switch (esc)
        {
        case '"':
        case '\\':
        case '/':
            out.push_back(esc);
            break;
        case 'b':
            out.push_back('\b');
            break;
        case 'f':
            out.push_back('\f');
            break;
        case 'n':
            out.push_back('\n');
            break;
        case 'r':
            out.push_back('\r');
            break;
        case 't':
            out.push_back('\t');
            break;
        case 'u':
            if (pos + 4 > text.size())
                return false;
            out.push_back('?');
            pos += 4;
            break;
        default:
            return false;
        }
    }
    return false;
}

void SkipJsonValue(const std::string& text, std::size_t& pos)
{
    if (!SkipWhitespace(text, pos) || pos >= text.size())
        return;

    if (text[pos] == '{')
    {
        std::size_t closePos = 0;
        if (FindMatchingBracket(text, pos, '{', '}', closePos))
            pos = closePos + 1;
        return;
    }

    if (text[pos] == '[')
    {
        std::size_t closePos = 0;
        if (FindMatchingBracket(text, pos, '[', ']', closePos))
            pos = closePos + 1;
        return;
    }

    if (text[pos] == '"')
    {
        std::string ignored;
        ReadJsonString(text, pos, ignored);
        return;
    }

    while (pos < text.size() && text[pos] != ',' && text[pos] != '}' && text[pos] != ']')
        ++pos;
}

bool FindObjectMemberValue(const std::string& json, std::size_t objectBegin, std::size_t objectEnd,
                           const std::string& key, std::size_t& valuePos)
{
    if (objectBegin >= objectEnd || json[objectBegin] != '{')
        return false;

    std::size_t pos = objectBegin + 1;
    while (pos < objectEnd)
    {
        SkipWhitespace(json, pos);
        if (pos >= objectEnd || json[pos] == '}')
            return false;

        std::string memberKey;
        if (!ReadJsonString(json, pos, memberKey))
            return false;

        SkipWhitespace(json, pos);
        if (pos >= objectEnd || json[pos] != ':')
            return false;
        ++pos;
        SkipWhitespace(json, pos);

        if (memberKey == key)
        {
            valuePos = pos;
            return true;
        }

        SkipJsonValue(json, pos);
        SkipWhitespace(json, pos);
        if (pos < objectEnd && json[pos] == ',')
            ++pos;
    }

    return false;
}

bool FindMatchingBracket(const std::string& text, std::size_t openPos, char openChar, char closeChar,
                         std::size_t& closePos)
{
    if (openPos >= text.size() || text[openPos] != openChar)
        return false;

    int depth = 0;
    bool inString = false;
    bool escaped = false;
    for (std::size_t pos = openPos; pos < text.size(); ++pos)
    {
        const char c = text[pos];
        if (inString)
        {
            if (escaped)
            {
                escaped = false;
            }
            else if (c == '\\')
            {
                escaped = true;
            }
            else if (c == '"')
            {
                inString = false;
            }
            continue;
        }

        if (c == '"')
        {
            inString = true;
        }
        else if (c == openChar)
        {
            ++depth;
        }
        else if (c == closeChar)
        {
            --depth;
            if (depth == 0)
            {
                closePos = pos;
                return true;
            }
        }
    }
    return false;
}

bool FindJsonPropertyValue(const std::string& text, const std::string& key, std::size_t begin, std::size_t end,
                           std::size_t& valuePos)
{
    std::size_t pos = begin;
    while (pos < end)
    {
        if (text[pos] != '"')
        {
            ++pos;
            continue;
        }

        const std::size_t keyStart = pos;
        std::string currentKey;
        if (!ReadJsonString(text, pos, currentKey))
            return false;

        std::size_t colon = pos;
        if (!SkipWhitespace(text, colon) || colon >= end || text[colon] != ':')
            continue;

        if (currentKey != key)
            continue;

        valuePos = colon + 1;
        SkipWhitespace(text, valuePos);
        return valuePos < end;
    }
    return false;
}

bool FindTopLevelBlock(const std::string& json, const std::string& key, char openChar, char closeChar,
                       std::size_t& blockBegin, std::size_t& blockEnd)
{
    std::size_t valuePos = 0;
    if (!FindJsonPropertyValue(json, key, 0, json.size(), valuePos))
        return false;
    if (valuePos >= json.size() || json[valuePos] != openChar)
        return false;

    std::size_t closePos = 0;
    if (!FindMatchingBracket(json, valuePos, openChar, closeChar, closePos))
        return false;

    blockBegin = valuePos;
    blockEnd = closePos + 1;
    return true;
}

void PushUnique(std::vector<std::string>& values, std::unordered_set<std::string>& seen, std::string value)
{
    if (value.empty())
        return;
    if (seen.insert(value).second)
        values.push_back(std::move(value));
}

void CollectObjectKeys(const std::string& json, std::size_t blockBegin, std::size_t blockEnd,
                       std::vector<std::string>& out)
{
    std::unordered_set<std::string> seen(out.begin(), out.end());
    std::size_t pos = blockBegin + 1;
    while (pos + 1 < blockEnd)
    {
        SkipWhitespace(json, pos);
        if (pos >= blockEnd || json[pos] == '}')
            break;

        std::string key;
        if (!ReadJsonString(json, pos, key))
            break;

        SkipWhitespace(json, pos);
        if (pos >= blockEnd || json[pos] != ':')
            break;
        ++pos;

        PushUnique(out, seen, std::move(key));

        SkipWhitespace(json, pos);
        if (pos >= blockEnd)
            break;

        if (json[pos] == '{')
        {
            std::size_t closePos = 0;
            if (!FindMatchingBracket(json, pos, '{', '}', closePos))
                break;
            pos = closePos + 1;
        }
        else if (json[pos] == '[')
        {
            std::size_t closePos = 0;
            if (!FindMatchingBracket(json, pos, '[', ']', closePos))
                break;
            pos = closePos + 1;
        }
        else if (json[pos] == '"')
        {
            std::string ignored;
            if (!ReadJsonString(json, pos, ignored))
                break;
        }
        else
        {
            while (pos < blockEnd && json[pos] != ',' && json[pos] != '}')
                ++pos;
        }

        SkipWhitespace(json, pos);
        if (pos < blockEnd && json[pos] == ',')
            ++pos;
    }
}

bool ReadStringPropertyInObject(const std::string& json, std::size_t objectBegin, std::size_t objectEnd,
                                const std::string& key, std::string& value)
{
    std::size_t valuePos = 0;
    if (!FindObjectMemberValue(json, objectBegin, objectEnd, key, valuePos))
        return false;
    return ReadJsonString(json, valuePos, value);
}

bool ReadFloatPropertyInObject(const std::string& json, std::size_t objectBegin, std::size_t objectEnd,
                               const std::string& key, float& value)
{
    std::size_t valuePos = 0;
    if (!FindObjectMemberValue(json, objectBegin, objectEnd, key, valuePos))
        return false;

    const char* begin = json.c_str() + valuePos;
    char* parseEnd = nullptr;
    const double parsed = std::strtod(begin, &parseEnd);
    if (parseEnd == begin)
        return false;

    value = static_cast<float>(parsed);
    return true;
}

bool ReadFloatArrayPropertyInObject(const std::string& json, std::size_t objectBegin, std::size_t objectEnd,
                                    const std::string& key, std::vector<float>& values)
{
    std::size_t valuePos = 0;
    if (!FindObjectMemberValue(json, objectBegin, objectEnd, key, valuePos) || json[valuePos] != '[')
        return false;

    std::size_t arrayEnd = 0;
    if (!FindMatchingBracket(json, valuePos, '[', ']', arrayEnd))
        return false;

    values.clear();
    std::size_t pos = valuePos + 1;
    while (pos < arrayEnd)
    {
        SkipWhitespace(json, pos);
        if (pos >= arrayEnd || json[pos] == ']')
            break;

        const char* begin = json.c_str() + pos;
        char* parseEnd = nullptr;
        const double parsed = std::strtod(begin, &parseEnd);
        if (parseEnd == begin)
            break;
        values.push_back(static_cast<float>(parsed));
        pos = static_cast<std::size_t>(parseEnd - json.c_str());

        SkipWhitespace(json, pos);
        if (pos < arrayEnd && json[pos] == ',')
            ++pos;
    }
    return true;
}

bool ReadIndexArrayPropertyInObject(const std::string& json, std::size_t objectBegin, std::size_t objectEnd,
                                    const std::string& key, std::vector<unsigned short>& values)
{
    std::vector<float> floats;
    if (!ReadFloatArrayPropertyInObject(json, objectBegin, objectEnd, key, floats))
        return false;

    values.clear();
    for (float value : floats)
    {
        if (value < 0.0f || value > 65535.0f)
            continue;
        values.push_back(static_cast<unsigned short>(value));
    }
    return true;
}

bool ReadStringArrayPropertyInObject(const std::string& json, std::size_t objectBegin, std::size_t objectEnd,
                                     const std::string& key, std::vector<std::string>& values)
{
    std::size_t valuePos = 0;
    if (!FindObjectMemberValue(json, objectBegin, objectEnd, key, valuePos) || json[valuePos] != '[')
        return false;

    std::size_t arrayEnd = 0;
    if (!FindMatchingBracket(json, valuePos, '[', ']', arrayEnd))
        return false;

    values.clear();
    std::size_t pos = valuePos + 1;
    while (pos < arrayEnd)
    {
        SkipWhitespace(json, pos);
        if (pos >= arrayEnd || json[pos] == ']')
            break;

        std::string value;
        if (!ReadJsonString(json, pos, value))
            break;
        values.push_back(std::move(value));

        SkipWhitespace(json, pos);
        if (pos < arrayEnd && json[pos] == ',')
            ++pos;
    }
    return true;
}

void ForEachObjectInArray(const std::string& json, std::size_t blockBegin, std::size_t blockEnd,
                          const std::function<void(std::size_t, std::size_t)>& callback)
{
    std::size_t pos = blockBegin + 1;
    while (pos + 1 < blockEnd)
    {
        SkipWhitespace(json, pos);
        if (pos >= blockEnd || json[pos] == ']')
            break;

        if (json[pos] != '{')
        {
            SkipJsonValue(json, pos);
            if (pos < blockEnd && json[pos] == ',')
                ++pos;
            continue;
        }

        std::size_t objectEnd = 0;
        if (!FindMatchingBracket(json, pos, '{', '}', objectEnd))
            break;

        callback(pos, objectEnd);
        pos = objectEnd + 1;
        SkipWhitespace(json, pos);
        if (pos < blockEnd && json[pos] == ',')
            ++pos;
    }
}

void ForEachObjectMember(const std::string& json, std::size_t objectBegin, std::size_t objectEnd,
                         const std::function<void(const std::string&, std::size_t, std::size_t)>& callback)
{
    std::size_t pos = objectBegin + 1;
    while (pos < objectEnd)
    {
        SkipWhitespace(json, pos);
        if (pos >= objectEnd || json[pos] == '}')
            break;

        std::string key;
        if (!ReadJsonString(json, pos, key))
            break;

        SkipWhitespace(json, pos);
        if (pos >= objectEnd || json[pos] != ':')
            break;
        ++pos;
        SkipWhitespace(json, pos);

        if (pos < objectEnd && json[pos] == '{')
        {
            std::size_t valueEnd = 0;
            if (!FindMatchingBracket(json, pos, '{', '}', valueEnd))
                break;
            callback(key, pos, valueEnd);
            pos = valueEnd + 1;
        }
        else
        {
            SkipJsonValue(json, pos);
        }

        SkipWhitespace(json, pos);
        if (pos < objectEnd && json[pos] == ',')
            ++pos;
    }
}

void CollectNamedObjectsInArray(const std::string& json, std::size_t blockBegin, std::size_t blockEnd,
                                std::vector<std::string>& out)
{
    std::unordered_set<std::string> seen(out.begin(), out.end());
    std::size_t pos = blockBegin + 1;
    while (pos + 1 < blockEnd)
    {
        SkipWhitespace(json, pos);
        if (pos >= blockEnd || json[pos] == ']')
            break;

        if (json[pos] != '{')
        {
            ++pos;
            continue;
        }

        std::size_t objectEnd = 0;
        if (!FindMatchingBracket(json, pos, '{', '}', objectEnd))
            break;

        std::string name;
        if (ReadStringPropertyInObject(json, pos, objectEnd, "name", name))
            PushUnique(out, seen, std::move(name));

        pos = objectEnd + 1;
        SkipWhitespace(json, pos);
        if (pos < blockEnd && json[pos] == ',')
            ++pos;
    }
}

std::vector<std::string> ParseAtlasPages(const std::string& atlasText)
{
    std::vector<std::string> pages;
    bool expectPage = true;

    std::size_t lineStart = 0;
    while (lineStart <= atlasText.size())
    {
        std::size_t lineEnd = atlasText.find('\n', lineStart);
        if (lineEnd == std::string::npos)
            lineEnd = atlasText.size();

        std::string line = Trim(atlasText.substr(lineStart, lineEnd - lineStart));
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty())
        {
            expectPage = true;
        }
        else if (expectPage && line.find(':') == std::string::npos)
        {
            pages.push_back(std::move(line));
            expectPage = false;
        }

        if (lineEnd == atlasText.size())
            break;
        lineStart = lineEnd + 1;
    }

    return pages;
}

float DegToRad(float deg)
{
    return deg * 3.14159265358979323846f / 180.0f;
}

struct Mat2D
{
    float a = 1.0f;
    float b = 0.0f;
    float c = 0.0f;
    float d = 1.0f;
    float tx = 0.0f;
    float ty = 0.0f;
    float worldScaleX = 1.0f;
    float worldScaleY = 1.0f;
    float worldRotation = 0.0f;
};

Mat2D MakeLocalTransform(float x, float y, float rotation, float scaleX, float scaleY)
{
    const float radians = DegToRad(rotation);
    const float cosValue = std::cos(radians);
    const float sinValue = std::sin(radians);

    Mat2D out{};
    out.a = cosValue * scaleX;
    out.b = sinValue * scaleX;
    out.c = -sinValue * scaleY;
    out.d = cosValue * scaleY;
    out.tx = x;
    out.ty = y;
    out.worldScaleX = scaleX;
    out.worldScaleY = scaleY;
    out.worldRotation = rotation;
    return out;
}

Mat2D Multiply(const Mat2D& parent, const Mat2D& child)
{
    Mat2D out{};
    out.a = parent.a * child.a + parent.c * child.b;
    out.b = parent.b * child.a + parent.d * child.b;
    out.c = parent.a * child.c + parent.c * child.d;
    out.d = parent.b * child.c + parent.d * child.d;
    out.tx = parent.a * child.tx + parent.c * child.ty + parent.tx;
    out.ty = parent.b * child.tx + parent.d * child.ty + parent.ty;
    return out;
}

Mat2D Inverse(const Mat2D& mat)
{
    Mat2D out{};
    const float det = mat.a * mat.d - mat.b * mat.c;
    if (std::fabs(det) < 0.000001f)
        return out;

    const float invDet = 1.0f / det;
    out.a = mat.d * invDet;
    out.b = -mat.b * invDet;
    out.c = -mat.c * invDet;
    out.d = mat.a * invDet;
    out.tx = -(out.a * mat.tx + out.c * mat.ty);
    out.ty = -(out.b * mat.tx + out.d * mat.ty);
    return out;
}

Vertex TransformVertex(const Mat2D& mat, float x, float y, float u, float v, const Color& color)
{
    Vertex out{};
    out.x = mat.a * x + mat.c * y + mat.tx;
    out.y = mat.b * x + mat.d * y + mat.ty;
    out.u = u;
    out.v = v;
    out.color = color;
    return out;
}

void TransformPoint(const Mat2D& mat, float x, float y, float& outX, float& outY)
{
    outX = mat.a * x + mat.c * y + mat.tx;
    outY = mat.b * x + mat.d * y + mat.ty;
}

Color ParseHexColor(const std::string& value)
{
    Color color{};
    if (value.size() < 8)
        return color;

    const auto readByte = [&value](std::size_t offset) -> float
    {
        const std::string part = value.substr(offset, 2);
        return static_cast<float>(std::strtoul(part.c_str(), nullptr, 16)) / 255.0f;
    };

    color.r = readByte(0);
    color.g = readByte(2);
    color.b = readByte(4);
    color.a = readByte(6);
    return color;
}

struct BoneData
{
    std::string name;
    std::string parentName;
    int parent = -1;
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float length = 0.0f;
    bool inheritScale = true;
    bool inheritRotation = true;
};

struct SlotData
{
    std::string name;
    std::string boneName;
    int bone = -1;
    std::string attachment;
    Color color;
    BlendMode blend = BlendMode::Normal;
};

struct IkConstraintData
{
    std::string name;
    std::vector<std::string> boneNames;
    std::vector<int> bones;
    std::string targetName;
    int target = -1;
    float mix = 1.0f;
    int bendDirection = 1;
};

struct AtlasRegion
{
    unsigned long long textureId = 0;
    float u = 0.0f;
    float v = 0.0f;
    float u2 = 1.0f;
    float v2 = 1.0f;
    float width = 0.0f;
    float height = 0.0f;
    float originalWidth = 0.0f;
    float originalHeight = 0.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    bool rotate = false;
};

struct RegionAttachment
{
    std::string slotName;
    std::string name;
    std::string path;
    bool mesh = false;
    bool skinnedMesh = false;
    float x = 0.0f;
    float y = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float rotation = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    Color color;
    std::vector<float> vertices;
    std::vector<float> uvs;
    std::vector<unsigned short> indices;
    struct WeightedBone
    {
        int bone = -1;
        float x = 0.0f;
        float y = 0.0f;
        float weight = 0.0f;
    };
    std::vector<std::vector<WeightedBone>> weightedVertices;
};

class BinaryReader
{
public:
    explicit BinaryReader(const std::string& data)
        : m_begin(reinterpret_cast<const unsigned char*>(data.data()))
        , m_pos(m_begin)
        , m_end(m_begin + data.size())
    {}

    bool Ok() const noexcept { return m_ok; }

    unsigned char U8()
    {
        if (m_pos >= m_end)
        {
            m_ok = false;
            return 0;
        }
        return *m_pos++;
    }

    signed char S8()
    {
        return static_cast<signed char>(U8());
    }

    bool Bool()
    {
        return U8() != 0;
    }

    int Int32()
    {
        const unsigned int a = U8();
        const unsigned int b = U8();
        const unsigned int c = U8();
        const unsigned int d = U8();
        return static_cast<int>((a << 24) | (b << 16) | (c << 8) | d);
    }

    int VarInt(bool optimizePositive)
    {
        unsigned char b = U8();
        int value = b & 0x7F;
        if (b & 0x80)
        {
            b = U8();
            value |= (b & 0x7F) << 7;
            if (b & 0x80)
            {
                b = U8();
                value |= (b & 0x7F) << 14;
                if (b & 0x80)
                {
                    b = U8();
                    value |= (b & 0x7F) << 21;
                    if (b & 0x80)
                        value |= (U8() & 0x7F) << 28;
                }
            }
        }

        if (!optimizePositive)
            value = static_cast<int>((static_cast<unsigned int>(value) >> 1) ^ -static_cast<unsigned int>(value & 1));
        return value;
    }

    float Float()
    {
        const unsigned int bits = static_cast<unsigned int>(Int32());
        float value = 0.0f;
        static_assert(sizeof(bits) == sizeof(value), "float must be 32 bit");
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }

    std::string String()
    {
        int charCount = VarInt(true);
        if (charCount == 0)
            return std::string();
        if (charCount == 1)
            return std::string();
        --charCount;

        const unsigned char* start = m_pos;
        for (int i = 0; i < charCount && m_pos < m_end; ++i)
        {
            const unsigned char b = *m_pos++;
            if (b > 127)
            {
                if ((b >> 5) == 0x6)
                    m_pos += 1;
                else if ((b >> 4) == 0xE)
                    m_pos += 2;
                else if ((b >> 3) == 0x1E)
                    m_pos += 3;
            }
        }

        if (m_pos > m_end)
        {
            m_ok = false;
            return std::string();
        }

        return std::string(reinterpret_cast<const char*>(start), static_cast<std::size_t>(m_pos - start));
    }

    std::vector<float> FloatArray(float scale)
    {
        const int count = VarInt(true);
        std::vector<float> values;
        if (count <= 0)
            return values;
        values.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i)
            values.push_back(Float() * scale);
        return values;
    }

    std::vector<unsigned short> ShortArray()
    {
        const int count = VarInt(true);
        std::vector<unsigned short> values;
        if (count <= 0)
            return values;
        values.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i)
        {
            const unsigned int hi = U8();
            const unsigned int lo = U8();
            values.push_back(static_cast<unsigned short>((hi << 8) | lo));
        }
        return values;
    }

    void SkipIntArray()
    {
        const int count = VarInt(true);
        for (int i = 0; i < count; ++i)
            VarInt(true);
    }

private:
    const unsigned char* m_begin = nullptr;
    const unsigned char* m_pos = nullptr;
    const unsigned char* m_end = nullptr;
    bool m_ok = true;
};

Color DecodePackedColor(int packed)
{
    Color out{};
    out.r = static_cast<float>((static_cast<unsigned int>(packed) & 0xff000000u) >> 24) / 255.0f;
    out.g = static_cast<float>((static_cast<unsigned int>(packed) & 0x00ff0000u) >> 16) / 255.0f;
    out.b = static_cast<float>((static_cast<unsigned int>(packed) & 0x0000ff00u) >> 8) / 255.0f;
    out.a = static_cast<float>(static_cast<unsigned int>(packed) & 0x000000ffu) / 255.0f;
    return out;
}

enum class CurveMode
{
    Linear,
    Stepped,
    Bezier,
};

struct FloatFrame
{
    float time = 0.0f;
    float a = 0.0f;
    float b = 0.0f;
    CurveMode curve = CurveMode::Linear;
    float cx1 = 0.0f;
    float cy1 = 0.0f;
    float cx2 = 1.0f;
    float cy2 = 1.0f;
};

struct ColorFrame
{
    float time = 0.0f;
    Color color;
    CurveMode curve = CurveMode::Linear;
    float cx1 = 0.0f;
    float cy1 = 0.0f;
    float cx2 = 1.0f;
    float cy2 = 1.0f;
};

struct AttachmentFrame
{
    float time = 0.0f;
    std::string name;
};

struct DrawOrderFrame
{
    float time = 0.0f;
    std::vector<int> order;
};

struct BoneTimeline
{
    std::vector<FloatFrame> rotate;
    std::vector<FloatFrame> translate;
    std::vector<FloatFrame> scale;
};

struct SlotTimeline
{
    std::vector<ColorFrame> color;
    std::vector<AttachmentFrame> attachment;
};

struct FfdFrame
{
    float time = 0.0f;
    std::vector<float> vertices;
    CurveMode curve = CurveMode::Linear;
    float cx1 = 0.0f;
    float cy1 = 0.0f;
    float cx2 = 1.0f;
    float cy2 = 1.0f;
};

struct FfdTimeline
{
    int slot = -1;
    std::string attachmentName;
    std::vector<FfdFrame> frames;
};

struct AnimationData
{
    std::string name;
    float duration = 0.0f;
    std::map<int, BoneTimeline> bones;
    std::map<int, SlotTimeline> slots;
    std::vector<DrawOrderFrame> drawOrder;
    std::vector<FfdTimeline> ffd;
};

struct TrackState
{
    std::string animation;
    float time = 0.0f;
    bool loop = true;
};

struct BonePose
{
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
};

struct SlotPose
{
    std::string attachment;
    Color color;
    std::vector<float> ffd;
};

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float NormalizeTimelineTime(float time, float duration, bool loop)
{
    if (!loop || duration <= 0.0f)
        return time;
    time = std::fmod(time, duration);
    return time < 0.0f ? time + duration : time;
}

float Cubic(float a, float b, float c, float d, float t)
{
    const float inv = 1.0f - t;
    return inv * inv * inv * a + 3.0f * inv * inv * t * b + 3.0f * inv * t * t * c + t * t * t * d;
}

float BezierPercent(float x, float cx1, float cy1, float cx2, float cy2)
{
    float lo = 0.0f;
    float hi = 1.0f;
    for (int i = 0; i < 12; ++i)
    {
        const float mid = (lo + hi) * 0.5f;
        const float curveX = Cubic(0.0f, cx1, cx2, 1.0f, mid);
        if (curveX < x)
            lo = mid;
        else
            hi = mid;
    }

    const float t = (lo + hi) * 0.5f;
    return Cubic(0.0f, cy1, cy2, 1.0f, t);
}

float FramePercent(float time, float aTime, float bTime, CurveMode curve, float cx1, float cy1, float cx2, float cy2)
{
    if (curve == CurveMode::Stepped || bTime <= aTime)
        return 0.0f;

    float percent = (time - aTime) / (bTime - aTime);
    if (percent < 0.0f)
        return 0.0f;
    if (percent > 1.0f)
        return 1.0f;

    if (curve == CurveMode::Bezier)
        return BezierPercent(percent, cx1, cy1, cx2, cy2);
    return percent;
}

FloatFrame SampleFloatFrame(const std::vector<FloatFrame>& frames, float time)
{
    if (frames.empty())
        return FloatFrame{};
    if (time <= frames.front().time)
        return frames.front();

    for (std::size_t i = 1; i < frames.size(); ++i)
    {
        if (time < frames[i].time)
        {
            const FloatFrame& prev = frames[i - 1];
            const FloatFrame& next = frames[i];
            const float percent = FramePercent(time, prev.time, next.time, prev.curve, prev.cx1, prev.cy1, prev.cx2, prev.cy2);
            FloatFrame out{};
            out.time = time;
            out.a = Lerp(prev.a, next.a, percent);
            out.b = Lerp(prev.b, next.b, percent);
            return out;
        }
    }

    return frames.back();
}

float NormalizeAngle(float value)
{
    while (value > 180.0f)
        value -= 360.0f;
    while (value < -180.0f)
        value += 360.0f;
    return value;
}

float SampleAngleFrame(const std::vector<FloatFrame>& frames, float time)
{
    if (frames.empty())
        return 0.0f;
    if (time <= frames.front().time)
        return frames.front().a;

    for (std::size_t i = 1; i < frames.size(); ++i)
    {
        if (time < frames[i].time)
        {
            const FloatFrame& prev = frames[i - 1];
            const FloatFrame& next = frames[i];
            const float percent = FramePercent(time, prev.time, next.time, prev.curve, prev.cx1, prev.cy1, prev.cx2, prev.cy2);
            const float delta = NormalizeAngle(next.a - prev.a);
            return prev.a + delta * percent;
        }
    }

    return frames.back().a;
}

Color SampleColorFrame(const std::vector<ColorFrame>& frames, float time)
{
    if (frames.empty())
        return Color{};
    if (time <= frames.front().time)
        return frames.front().color;

    for (std::size_t i = 1; i < frames.size(); ++i)
    {
        if (time < frames[i].time)
        {
            const ColorFrame& prev = frames[i - 1];
            const ColorFrame& next = frames[i];
            const float percent = FramePercent(time, prev.time, next.time, prev.curve, prev.cx1, prev.cy1, prev.cx2, prev.cy2);
            Color out{};
            out.r = Lerp(prev.color.r, next.color.r, percent);
            out.g = Lerp(prev.color.g, next.color.g, percent);
            out.b = Lerp(prev.color.b, next.color.b, percent);
            out.a = Lerp(prev.color.a, next.color.a, percent);
            return out;
        }
    }

    return frames.back().color;
}

std::string SampleAttachmentFrame(const std::vector<AttachmentFrame>& frames, float time, const std::string& fallback)
{
    if (frames.empty())
        return fallback;

    std::string value = fallback;
    for (const AttachmentFrame& frame : frames)
    {
        if (frame.time > time)
            break;
        value = frame.name;
    }
    return value;
}

std::vector<int> SampleDrawOrderFrame(const std::vector<DrawOrderFrame>& frames, float time, std::size_t slotCount)
{
    std::vector<int> order(slotCount);
    for (std::size_t i = 0; i < slotCount; ++i)
        order[i] = static_cast<int>(i);

    for (const DrawOrderFrame& frame : frames)
    {
        if (frame.time > time)
            break;
        if (frame.order.size() == slotCount)
            order = frame.order;
    }
    return order;
}

std::vector<float> SampleFfdFrame(const std::vector<FfdFrame>& frames, float time)
{
    if (frames.empty())
        return {};
    if (time <= frames.front().time)
        return frames.front().vertices;

    for (std::size_t i = 1; i < frames.size(); ++i)
    {
        if (time < frames[i].time)
        {
            const FfdFrame& prev = frames[i - 1];
            const FfdFrame& next = frames[i];
            const float percent = FramePercent(time, prev.time, next.time, prev.curve, prev.cx1, prev.cy1, prev.cx2, prev.cy2);
            const std::size_t count = std::min(prev.vertices.size(), next.vertices.size());
            std::vector<float> out(count);
            for (std::size_t n = 0; n < count; ++n)
                out[n] = Lerp(prev.vertices[n], next.vertices[n], percent);
            return out;
        }
    }

    return frames.back().vertices;
}

template <typename TFrame>
void ReadCurve(const std::string& jsonText, std::size_t objectBegin, std::size_t objectEnd, TFrame& frame)
{
    std::string curve;
    if (ReadStringPropertyInObject(jsonText, objectBegin, objectEnd, "curve", curve) && curve == "stepped")
    {
        frame.curve = CurveMode::Stepped;
        return;
    }

    std::vector<float> curveValues;
    if (ReadFloatArrayPropertyInObject(jsonText, objectBegin, objectEnd, "curve", curveValues) && curveValues.size() >= 4)
    {
        frame.curve = CurveMode::Bezier;
        frame.cx1 = curveValues[0];
        frame.cy1 = curveValues[1];
        frame.cx2 = curveValues[2];
        frame.cy2 = curveValues[3];
    }
}

bool ParseFloatPair(const std::string& text, float& a, float& b)
{
    const std::size_t comma = text.find(',');
    if (comma == std::string::npos)
        return false;

    a = static_cast<float>(std::strtod(Trim(text.substr(0, comma)).c_str(), nullptr));
    b = static_cast<float>(std::strtod(Trim(text.substr(comma + 1)).c_str(), nullptr));
    return true;
}

void ParseAtlasRegions(const std::string& atlasText, const std::vector<unsigned long long>& pageIds,
                       std::map<std::string, AtlasRegion>& outRegions)
{
    struct PendingRegion
    {
        std::string name;
        unsigned long long textureId = 0;
        float pageWidth = 0.0f;
        float pageHeight = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        float originalWidth = 0.0f;
        float originalHeight = 0.0f;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        bool rotate = false;
        bool hasSize = false;
    };

    const auto flushRegion = [&outRegions](PendingRegion& region)
    {
        if (region.name.empty() || region.textureId == 0 || !region.hasSize || region.pageWidth <= 0.0f || region.pageHeight <= 0.0f)
            return;

        AtlasRegion atlas{};
        atlas.textureId = region.textureId;
        atlas.width = region.width;
        atlas.height = region.height;
        atlas.originalWidth = region.originalWidth > 0.0f ? region.originalWidth : region.width;
        atlas.originalHeight = region.originalHeight > 0.0f ? region.originalHeight : region.height;
        atlas.offsetX = region.offsetX;
        atlas.offsetY = region.offsetY;
        atlas.rotate = region.rotate;
        atlas.u = region.x / region.pageWidth;
        atlas.v = region.y / region.pageHeight;
        atlas.u2 = (region.x + (region.rotate ? region.height : region.width)) / region.pageWidth;
        atlas.v2 = (region.y + (region.rotate ? region.width : region.height)) / region.pageHeight;
        outRegions[region.name] = atlas;
    };

    std::size_t pageIndex = 0;
    unsigned long long currentTextureId = 0;
    float pageWidth = 0.0f;
    float pageHeight = 0.0f;
    bool expectPage = true;
    PendingRegion region{};

    std::size_t lineStart = 0;
    while (lineStart <= atlasText.size())
    {
        std::size_t lineEnd = atlasText.find('\n', lineStart);
        if (lineEnd == std::string::npos)
            lineEnd = atlasText.size();

        std::string line = Trim(atlasText.substr(lineStart, lineEnd - lineStart));
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty())
        {
            flushRegion(region);
            region = PendingRegion{};
            expectPage = true;
            pageWidth = 0.0f;
            pageHeight = 0.0f;
        }
        else if (line.find(':') == std::string::npos)
        {
            if (expectPage)
            {
                flushRegion(region);
                region = PendingRegion{};
                currentTextureId = pageIndex < pageIds.size() ? pageIds[pageIndex] : 0;
                ++pageIndex;
                expectPage = false;
            }
            else
            {
                flushRegion(region);
                region = PendingRegion{};
                region.name = line;
                region.textureId = currentTextureId;
                region.pageWidth = pageWidth;
                region.pageHeight = pageHeight;
            }
        }
        else
        {
            const std::size_t colon = line.find(':');
            const std::string key = Trim(line.substr(0, colon));
            const std::string value = Trim(line.substr(colon + 1));

            if (region.name.empty())
            {
                if (key == "size")
                    ParseFloatPair(value, pageWidth, pageHeight);
            }
            else if (key == "xy")
            {
                ParseFloatPair(value, region.x, region.y);
            }
            else if (key == "rotate")
            {
                std::string lower = value;
                std::transform(lower.begin(), lower.end(), lower.begin(),
                    [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
                region.rotate = lower == "true";
            }
            else if (key == "size")
            {
                region.hasSize = ParseFloatPair(value, region.width, region.height);
            }
            else if (key == "orig")
            {
                ParseFloatPair(value, region.originalWidth, region.originalHeight);
            }
            else if (key == "offset")
            {
                ParseFloatPair(value, region.offsetX, region.offsetY);
            }
        }

        if (lineEnd == atlasText.size())
            break;
        lineStart = lineEnd + 1;
    }

    flushRegion(region);
}

class Spine21CppRuntime final : public IRuntime
{
public:
    RuntimeInfo Info() const noexcept override
    {
        return RuntimeInfo{ RuntimeKind::Cpp21, "2.1", "2.1 C++" };
    }

    bool Load(const LoadRequest& request) override
    {
        Clear();

        std::string atlasText;
        std::string atlasBasePath;
        if (!LoadAtlasText(request, atlasText, atlasBasePath))
            return false;

        if (!LoadAtlasTextures(atlasText, atlasBasePath, request.textureDirectories))
            return false;

        if (request.binarySkeleton)
        {
            if (!LoadBinarySkeleton(request))
                return false;
            if (!m_animationNames.empty())
                StartMotion(m_animationNames.front().c_str(), true);
            if (m_currentSkin.empty() && !m_skinNames.empty())
                m_currentSkin = m_skinNames.front();
            m_hasSkeleton = true;
            m_lastError.clear();
            return true;
        }

        std::string jsonText;
        if (!LoadSkeletonJsonText(request, jsonText))
            return false;

        if (!LoadJsonMetadata(jsonText))
            return false;

        if (m_skinNames.empty())
            m_skinNames.push_back("default");

        if (!m_animationNames.empty())
            StartMotion(m_animationNames.front().c_str(), true);
        if (m_currentSkin.empty() && !m_skinNames.empty())
            m_currentSkin = m_skinNames.front();

        m_hasSkeleton = true;
        m_lastError.clear();
        return true;
    }

    void Clear() noexcept override
    {
        m_animationNames.clear();
        m_skinNames.clear();
        m_slotNames.clear();
        m_textureInfos.clear();
        m_bones.clear();
        m_slots.clear();
        m_ikConstraints.clear();
        m_atlasRegions.clear();
        m_attachments.clear();
        m_animations.clear();
        m_currentAnimation.clear();
        m_currentSkin.clear();
        m_additionalTracks.clear();
        m_currentTime = 0.0f;
        m_currentLoop = true;
        m_lastError.clear();
        m_hasSkeleton = false;
    }

    bool HasSkeleton() const noexcept override { return m_hasSkeleton; }
    void Update(float deltaSeconds) override
    {
        if (!m_currentAnimation.empty())
            m_currentTime += deltaSeconds;
        for (TrackState& track : m_additionalTracks)
            track.time += deltaSeconds;
    }
    void BuildFrame(int width, int height, Frame& outFrame) override
    {
        outFrame = Frame{};
        outFrame.width = width;
        outFrame.height = height;
        if (!m_hasSkeleton)
            return;

        std::vector<BonePose> bonePose;
        std::vector<SlotPose> slotPose;
        BuildSetupPose(bonePose, slotPose);
        ApplyCurrentAnimation(bonePose, slotPose);

        std::vector<Mat2D> boneWorld;
        BuildBoneWorld(bonePose, boneWorld);
        ApplyIkConstraints(bonePose, boneWorld);

        const std::vector<int> drawOrder = BuildCurrentDrawOrder();
        for (int orderedSlotIndex : drawOrder)
        {
            if (orderedSlotIndex < 0 || static_cast<std::size_t>(orderedSlotIndex) >= m_slots.size())
                continue;

            const std::size_t slotIndex = static_cast<std::size_t>(orderedSlotIndex);
            const SlotData& slot = m_slots[slotIndex];
            const SlotPose& pose = slotPose[slotIndex];
            if (slot.bone < 0 || static_cast<std::size_t>(slot.bone) >= boneWorld.size() || pose.attachment.empty())
                continue;

            const RegionAttachment* attachment = FindAttachment(slot.name, pose.attachment);
            if (attachment == nullptr)
                continue;

            const std::string regionKey = attachment->path.empty() ? attachment->name : attachment->path;
            const AtlasRegion* atlasRegion = FindAtlasRegion(regionKey);
            if (atlasRegion == nullptr)
                continue;

            const Mat2D attachmentLocal = MakeLocalTransform(attachment->x, attachment->y, attachment->rotation, 1.0f, 1.0f);
            const Mat2D transform = Multiply(boneWorld[static_cast<std::size_t>(slot.bone)], attachmentLocal);

            Color finalColor = pose.color;
            finalColor.r *= attachment->color.r;
            finalColor.g *= attachment->color.g;
            finalColor.b *= attachment->color.b;
            finalColor.a *= attachment->color.a;

            DrawCommand cmd{};
            cmd.textureId = atlasRegion->textureId;
            cmd.slotName = slot.name;
            cmd.blendMode = slot.blend;
            cmd.premultipliedAlpha = true;

            if (attachment->skinnedMesh && !attachment->weightedVertices.empty() && attachment->uvs.size() >= 2 && !attachment->indices.empty())
            {
                const std::size_t vertexCount = std::min(attachment->weightedVertices.size(), attachment->uvs.size() / 2);
                cmd.vertices.reserve(vertexCount);
                std::size_t ffdIndex = 0;
                for (std::size_t i = 0; i < vertexCount; ++i)
                {
                    float x = 0.0f;
                    float y = 0.0f;
                    for (const RegionAttachment::WeightedBone& weight : attachment->weightedVertices[i])
                    {
                        if (weight.bone < 0 || static_cast<std::size_t>(weight.bone) >= boneWorld.size())
                            continue;

                        float localX = weight.x;
                        float localY = weight.y;
                        if (ffdIndex + 1 < pose.ffd.size())
                        {
                            localX += pose.ffd[ffdIndex];
                            localY += pose.ffd[ffdIndex + 1];
                        }
                        ffdIndex += 2;

                        float wx = 0.0f;
                        float wy = 0.0f;
                        TransformPoint(boneWorld[static_cast<std::size_t>(weight.bone)], localX, localY, wx, wy);
                        x += wx * weight.weight;
                        y += wy * weight.weight;
                    }

                    const float u = attachment->uvs[i * 2 + 0];
                    const float v = attachment->uvs[i * 2 + 1];
                    Vertex vertex{};
                    vertex.x = x;
                    vertex.y = y;
                    vertex.u = u;
                    vertex.v = v;
                    vertex.color = finalColor;
                    cmd.vertices.push_back(vertex);
                }
                cmd.indices = attachment->indices;
            }
            else if (attachment->mesh && attachment->vertices.size() >= 2 && attachment->uvs.size() >= 2 && !attachment->indices.empty())
            {
                const std::size_t vertexCount = std::min(attachment->vertices.size(), attachment->uvs.size()) / 2;
                cmd.vertices.reserve(vertexCount);
                for (std::size_t i = 0; i < vertexCount; ++i)
                {
                    float x = attachment->vertices[i * 2 + 0];
                    float y = attachment->vertices[i * 2 + 1];
                    if (i * 2 + 1 < pose.ffd.size())
                    {
                        x = pose.ffd[i * 2 + 0];
                        y = pose.ffd[i * 2 + 1];
                    }
                    const float u = attachment->uvs[i * 2 + 0];
                    const float v = attachment->uvs[i * 2 + 1];
                    cmd.vertices.push_back(TransformVertex(transform, x, y, u, v, finalColor));
                }
                cmd.indices = attachment->indices;
            }
            else
            {
                cmd.indices = { 0, 1, 2, 2, 3, 0 };

                const float sourceWidth = attachment->width > 0.0f ? attachment->width : atlasRegion->originalWidth;
                const float sourceHeight = attachment->height > 0.0f ? attachment->height : atlasRegion->originalHeight;
                const float originalWidth = atlasRegion->originalWidth > 0.0f ? atlasRegion->originalWidth : sourceWidth;
                const float originalHeight = atlasRegion->originalHeight > 0.0f ? atlasRegion->originalHeight : sourceHeight;
                const float regionScaleX = originalWidth != 0.0f ? sourceWidth / originalWidth * attachment->scaleX : attachment->scaleX;
                const float regionScaleY = originalHeight != 0.0f ? sourceHeight / originalHeight * attachment->scaleY : attachment->scaleY;
                const float left = -sourceWidth * 0.5f * attachment->scaleX + atlasRegion->offsetX * regionScaleX;
                const float bottom = -sourceHeight * 0.5f * attachment->scaleY + atlasRegion->offsetY * regionScaleY;
                const float right = left + atlasRegion->width * regionScaleX;
                const float top = bottom + atlasRegion->height * regionScaleY;

                if (atlasRegion->rotate)
                {
                    cmd.vertices.push_back(TransformVertex(transform, left, bottom, atlasRegion->u2, atlasRegion->v2, finalColor));
                    cmd.vertices.push_back(TransformVertex(transform, left, top, atlasRegion->u, atlasRegion->v2, finalColor));
                    cmd.vertices.push_back(TransformVertex(transform, right, top, atlasRegion->u, atlasRegion->v, finalColor));
                    cmd.vertices.push_back(TransformVertex(transform, right, bottom, atlasRegion->u2, atlasRegion->v, finalColor));
                }
                else
                {
                    cmd.vertices.push_back(TransformVertex(transform, left, bottom, atlasRegion->u, atlasRegion->v2, finalColor));
                    cmd.vertices.push_back(TransformVertex(transform, left, top, atlasRegion->u, atlasRegion->v, finalColor));
                    cmd.vertices.push_back(TransformVertex(transform, right, top, atlasRegion->u2, atlasRegion->v, finalColor));
                    cmd.vertices.push_back(TransformVertex(transform, right, bottom, atlasRegion->u2, atlasRegion->v2, finalColor));
                }
            }

            if (!cmd.vertices.empty() && !cmd.indices.empty())
                outFrame.draws.push_back(std::move(cmd));
        }
    }

    const std::vector<std::string>& MotionNames() const noexcept override { return m_animationNames; }
    const std::vector<std::string>& LookNames() const noexcept override { return m_skinNames; }
    const std::vector<std::string>& SlotCatalog() const noexcept override { return m_slotNames; }
    const std::vector<TextureInfo>& TextureInfos() const noexcept override { return m_textureInfos; }

    void StartMotion(const char* name, bool loop) override
    {
        if (name == nullptr || name[0] == '\0')
        {
            m_currentAnimation.clear();
            m_currentTime = 0.0f;
            return;
        }

        const auto it = m_animations.find(name);
        if (it == m_animations.end())
            return;

        m_currentAnimation = name;
        m_currentTime = 0.0f;
        m_currentLoop = loop;
    }

    float MotionDuration(const char* name) const override
    {
        if (name == nullptr)
            return 0.0f;

        const auto it = m_animations.find(name);
        return it == m_animations.end() ? 0.0f : it->second.duration;
    }

    void SetMotionBlendSeconds(float) override {}

    void SetSecondaryMotions(const std::vector<std::string>& names, bool loop) override
    {
        std::vector<TrackState> nextTracks;
        nextTracks.reserve(names.size());
        for (const std::string& name : names)
        {
            if (name.empty() || m_animations.find(name) == m_animations.end())
                continue;

            TrackState state{};
            state.animation = name;
            state.time = m_currentTime;
            state.loop = loop;

            const auto existing = std::find_if(m_additionalTracks.begin(), m_additionalTracks.end(),
                [&name](const TrackState& track) { return track.animation == name; });
            if (existing != m_additionalTracks.end())
                state.time = existing->time;

            nextTracks.push_back(std::move(state));
        }
        m_additionalTracks = std::move(nextTracks);
    }
    void ApplyLook(const char* name) override
    {
        if (m_skinNames.empty())
            return;

        if (name == nullptr || name[0] == '\0')
        {
            m_currentSkin = m_skinNames.front();
            return;
        }

        const auto it = std::find(m_skinNames.begin(), m_skinNames.end(), std::string(name));
        if (it != m_skinNames.end())
            m_currentSkin = *it;
    }
    std::string LastError() const override { return m_lastError; }

private:
    bool LoadAtlasText(const LoadRequest& request, std::string& outText, std::string& outBasePath)
    {
        if (!request.atlasData.empty())
        {
            outText = request.atlasData.front();
            outBasePath.clear();
            return true;
        }

        if (request.atlasPaths.empty())
        {
            m_lastError = "Spine 2.1 load failed: atlas file is missing.";
            return false;
        }

        const std::string& atlasPath = request.atlasPaths.front();
        if (!ReadTextFile(atlasPath, outText))
        {
            m_lastError = "Spine 2.1 load failed: could not read atlas file: " + atlasPath;
            return false;
        }

        outBasePath = ParentPathOf(atlasPath);
        return true;
    }

    bool LoadAtlasTextures(const std::string& atlasText, const std::string& atlasBasePath,
                           const std::vector<std::string>& textureDirectories)
    {
        const std::vector<std::string> pages = ParseAtlasPages(atlasText);
        if (pages.empty())
        {
            m_lastError = "Spine 2.1 load failed: atlas has no texture pages.";
            return false;
        }

        unsigned long long nextId = 1;
        std::vector<unsigned long long> pageIds;
        for (const std::string& page : pages)
        {
            TextureInfo info{};
            info.id = nextId++;
            info.path = ResolveTexturePath(page, atlasBasePath, textureDirectories);
            info.sourcePremultipliedAlpha = true;
            info.renderPremultipliedAlpha = true;
            info.hasPremultipliedAlphaMetadata = false;
            pageIds.push_back(info.id);
            m_textureInfos.push_back(std::move(info));
        }
        ParseAtlasRegions(atlasText, pageIds, m_atlasRegions);
        return true;
    }

    std::string ResolveTexturePath(const std::string& pageName, const std::string& atlasBasePath,
                                   const std::vector<std::string>& textureDirectories) const
    {
        if (!textureDirectories.empty())
            return JoinPath(textureDirectories.front(), pageName);
        return JoinPath(atlasBasePath, pageName);
    }

    bool LoadSkeletonJsonText(const LoadRequest& request, std::string& outText)
    {
        if (!request.skeletonData.empty())
        {
            outText = request.skeletonData.front();
            return true;
        }

        if (request.skeletonPaths.empty())
        {
            m_lastError = "Spine 2.1 load failed: skeleton json is missing.";
            return false;
        }

        if (!ReadTextFile(request.skeletonPaths.front(), outText))
        {
            m_lastError = "Spine 2.1 load failed: could not read skeleton json: " + request.skeletonPaths.front();
            return false;
        }
        return true;
    }

    bool LoadSkeletonBinaryData(const LoadRequest& request, std::string& outData)
    {
        if (!request.skeletonData.empty())
        {
            outData = request.skeletonData.front();
            return true;
        }

        if (request.skeletonPaths.empty())
        {
            m_lastError = "Spine 2.1 binary load failed: skeleton data is missing.";
            return false;
        }

        if (!ReadTextFile(request.skeletonPaths.front(), outData))
        {
            m_lastError = "Spine 2.1 binary load failed: could not read skeleton file: " + request.skeletonPaths.front();
            return false;
        }
        return true;
    }

    bool LoadJsonMetadata(const std::string& jsonText)
    {
        std::size_t slotsBegin = 0;
        std::size_t slotsEnd = 0;
        if (FindTopLevelBlock(jsonText, "slots", '[', ']', slotsBegin, slotsEnd))
            CollectNamedObjectsInArray(jsonText, slotsBegin, slotsEnd, m_slotNames);

        std::size_t skinsBegin = 0;
        std::size_t skinsEnd = 0;
        if (FindTopLevelBlock(jsonText, "skins", '{', '}', skinsBegin, skinsEnd))
            CollectObjectKeys(jsonText, skinsBegin, skinsEnd, m_skinNames);

        std::size_t animationsBegin = 0;
        std::size_t animationsEnd = 0;
        if (FindTopLevelBlock(jsonText, "animations", '{', '}', animationsBegin, animationsEnd))
            CollectObjectKeys(jsonText, animationsBegin, animationsEnd, m_animationNames);

        LoadBones(jsonText);
        LoadSlots(jsonText);
        LoadIkConstraints(jsonText);
        LoadSkins(jsonText);
        ResolveBoneLinks();
        LoadAnimations(jsonText);

        if (m_slotNames.empty())
        {
            m_lastError = "Spine 2.1 load failed: json contains no slots.";
            return false;
        }

        return true;
    }

    void BuildSetupPose(std::vector<BonePose>& bones, std::vector<SlotPose>& slots) const
    {
        bones.clear();
        bones.reserve(m_bones.size());
        for (const BoneData& bone : m_bones)
        {
            BonePose pose{};
            pose.x = bone.x;
            pose.y = bone.y;
            pose.rotation = bone.rotation;
            pose.scaleX = bone.scaleX;
            pose.scaleY = bone.scaleY;
            bones.push_back(pose);
        }

        slots.clear();
        slots.reserve(m_slots.size());
        for (const SlotData& slot : m_slots)
        {
            SlotPose pose{};
            pose.attachment = slot.attachment;
            pose.color = slot.color;
            slots.push_back(std::move(pose));
        }
    }

    void ApplyCurrentAnimation(std::vector<BonePose>& bones, std::vector<SlotPose>& slots) const
    {
        const auto animationIt = m_animations.find(m_currentAnimation);
        if (animationIt != m_animations.end())
        {
            const AnimationData& animation = animationIt->second;
            const float time = NormalizeTimelineTime(m_currentTime, animation.duration, m_currentLoop);
            ApplyAnimationTimelines(animation, time, bones, slots);
        }

        for (const TrackState& track : m_additionalTracks)
        {
            const auto trackAnimationIt = m_animations.find(track.animation);
            if (trackAnimationIt == m_animations.end())
                continue;

            const AnimationData& animation = trackAnimationIt->second;
            const float time = NormalizeTimelineTime(track.time, animation.duration, track.loop);
            ApplyAnimationTimelines(animation, time, bones, slots);
        }
    }

    void ApplyAnimationTimelines(const AnimationData& animation, float time, std::vector<BonePose>& bones, std::vector<SlotPose>& slots) const
    {
        for (const auto& entry : animation.bones)
        {
            const int boneIndex = entry.first;
            if (boneIndex < 0 || static_cast<std::size_t>(boneIndex) >= bones.size())
                continue;

            BonePose& pose = bones[static_cast<std::size_t>(boneIndex)];
            const BoneTimeline& timeline = entry.second;

            if (!timeline.rotate.empty() && time >= timeline.rotate.front().time)
                pose.rotation = m_bones[static_cast<std::size_t>(boneIndex)].rotation + SampleAngleFrame(timeline.rotate, time);

            if (!timeline.translate.empty() && time >= timeline.translate.front().time)
            {
                const FloatFrame frame = SampleFloatFrame(timeline.translate, time);
                pose.x = m_bones[static_cast<std::size_t>(boneIndex)].x + frame.a;
                pose.y = m_bones[static_cast<std::size_t>(boneIndex)].y + frame.b;
            }

            if (!timeline.scale.empty() && time >= timeline.scale.front().time)
            {
                const FloatFrame frame = SampleFloatFrame(timeline.scale, time);
                pose.scaleX = m_bones[static_cast<std::size_t>(boneIndex)].scaleX * frame.a;
                pose.scaleY = m_bones[static_cast<std::size_t>(boneIndex)].scaleY * frame.b;
            }
        }

        for (const auto& entry : animation.slots)
        {
            const int slotIndex = entry.first;
            if (slotIndex < 0 || static_cast<std::size_t>(slotIndex) >= slots.size())
                continue;

            SlotPose& pose = slots[static_cast<std::size_t>(slotIndex)];
            const SlotTimeline& timeline = entry.second;
            if (!timeline.color.empty() && time >= timeline.color.front().time)
                pose.color = SampleColorFrame(timeline.color, time);
            if (!timeline.attachment.empty())
                pose.attachment = SampleAttachmentFrame(timeline.attachment, time, pose.attachment);
        }

        for (const FfdTimeline& timeline : animation.ffd)
        {
            if (timeline.slot < 0 || static_cast<std::size_t>(timeline.slot) >= slots.size())
                continue;
            SlotPose& pose = slots[static_cast<std::size_t>(timeline.slot)];
            if (pose.attachment != timeline.attachmentName || timeline.frames.empty() || time < timeline.frames.front().time)
                continue;
            pose.ffd = SampleFfdFrame(timeline.frames, time);
        }
    }

    void BuildBoneWorld(const std::vector<BonePose>& bonePose, std::vector<Mat2D>& boneWorld) const
    {
        boneWorld.assign(m_bones.size(), Mat2D{});
        for (std::size_t i = 0; i < m_bones.size(); ++i)
        {
            const BoneData& bone = m_bones[i];
            const BonePose& pose = bonePose[i];
            Mat2D world{};
            if (bone.parent >= 0 && static_cast<std::size_t>(bone.parent) < boneWorld.size())
            {
                const Mat2D& parent = boneWorld[static_cast<std::size_t>(bone.parent)];
                world.tx = pose.x * parent.a + pose.y * parent.c + parent.tx;
                world.ty = pose.x * parent.b + pose.y * parent.d + parent.ty;
                world.worldScaleX = bone.inheritScale ? parent.worldScaleX * pose.scaleX : pose.scaleX;
                world.worldScaleY = bone.inheritScale ? parent.worldScaleY * pose.scaleY : pose.scaleY;
                world.worldRotation = bone.inheritRotation ? parent.worldRotation + pose.rotation : pose.rotation;
            }
            else
            {
                world.tx = pose.x;
                world.ty = pose.y;
                world.worldScaleX = pose.scaleX;
                world.worldScaleY = pose.scaleY;
                world.worldRotation = pose.rotation;
            }

            const float radians = DegToRad(world.worldRotation);
            const float cosValue = std::cos(radians);
            const float sinValue = std::sin(radians);
            world.a = cosValue * world.worldScaleX;
            world.b = sinValue * world.worldScaleX;
            world.c = -sinValue * world.worldScaleY;
            world.d = cosValue * world.worldScaleY;
            boneWorld[i] = world;
        }
    }

    void ApplyIkConstraints(std::vector<BonePose>& bonePose, std::vector<Mat2D>& boneWorld) const
    {
        for (const IkConstraintData& ik : m_ikConstraints)
        {
            if (ik.bones.empty() || ik.target < 0 || static_cast<std::size_t>(ik.target) >= boneWorld.size())
                continue;

            if (ik.bones.size() == 1)
            {
                ApplyOneBoneIk(ik.bones[0], bonePose, boneWorld, boneWorld[static_cast<std::size_t>(ik.target)].tx,
                               boneWorld[static_cast<std::size_t>(ik.target)].ty, ik.mix);
            }
            else if (ik.bones.size() >= 2)
            {
                ApplyTwoBoneIk(ik.bones[0], ik.bones[1], bonePose, boneWorld,
                               boneWorld[static_cast<std::size_t>(ik.target)].tx,
                               boneWorld[static_cast<std::size_t>(ik.target)].ty,
                               ik.bendDirection, ik.mix);
            }
            BuildBoneWorld(bonePose, boneWorld);
        }
    }

    void ApplyOneBoneIk(int boneIndex, std::vector<BonePose>& bonePose, const std::vector<Mat2D>& boneWorld,
                        float targetX, float targetY, float alpha) const
    {
        if (boneIndex < 0 || static_cast<std::size_t>(boneIndex) >= m_bones.size())
            return;

        const BoneData& bone = m_bones[static_cast<std::size_t>(boneIndex)];
        const float parentRotation = (bone.parent >= 0 && bone.inheritRotation) ? boneWorld[static_cast<std::size_t>(bone.parent)].worldRotation : 0.0f;
        const float targetAngle = std::atan2(targetY - boneWorld[static_cast<std::size_t>(boneIndex)].ty,
                                             targetX - boneWorld[static_cast<std::size_t>(boneIndex)].tx) * 180.0f / 3.14159265358979323846f;
        const float desired = targetAngle - parentRotation;
        bonePose[static_cast<std::size_t>(boneIndex)].rotation += NormalizeAngle(desired - bonePose[static_cast<std::size_t>(boneIndex)].rotation) * alpha;
    }

    void ApplyTwoBoneIk(int parentIndex, int childIndex, std::vector<BonePose>& bonePose, const std::vector<Mat2D>& boneWorld,
                        float targetX, float targetY, int bendDirection, float alpha) const
    {
        if (parentIndex < 0 || childIndex < 0 ||
            static_cast<std::size_t>(parentIndex) >= m_bones.size() ||
            static_cast<std::size_t>(childIndex) >= m_bones.size())
            return;

        const BoneData& parentBone = m_bones[static_cast<std::size_t>(parentIndex)];
        const BoneData& childBone = m_bones[static_cast<std::size_t>(childIndex)];
        const Mat2D parentParentWorld = parentBone.parent >= 0 ? boneWorld[static_cast<std::size_t>(parentBone.parent)] : Mat2D{};
        const Mat2D parentParentInv = Inverse(parentParentWorld);

        float localTargetX = 0.0f;
        float localTargetY = 0.0f;
        TransformPoint(parentParentInv, targetX, targetY, localTargetX, localTargetY);
        localTargetX -= bonePose[static_cast<std::size_t>(parentIndex)].x;
        localTargetY -= bonePose[static_cast<std::size_t>(parentIndex)].y;

        const float childX = bonePose[static_cast<std::size_t>(childIndex)].x;
        const float childY = bonePose[static_cast<std::size_t>(childIndex)].y;
        const float offset = std::atan2(childY, childX);
        const float len1 = std::sqrt(childX * childX + childY * childY);
        const float len2 = childBone.length * std::fabs(boneWorld[static_cast<std::size_t>(childIndex)].worldScaleX);

        if (len1 < 0.0001f || len2 < 0.0001f)
        {
            ApplyOneBoneIk(parentIndex, bonePose, boneWorld, targetX, targetY, alpha);
            return;
        }

        float cosValue = (localTargetX * localTargetX + localTargetY * localTargetY - len1 * len1 - len2 * len2) / (2.0f * len1 * len2);
        cosValue = std::max(-1.0f, std::min(1.0f, cosValue));
        const float childAngle = std::acos(cosValue) * static_cast<float>(bendDirection == 0 ? 1 : bendDirection);
        const float adjacent = len1 + len2 * cosValue;
        const float opposite = len2 * std::sin(childAngle);
        const float parentAngle = std::atan2(localTargetY * adjacent - localTargetX * opposite,
                                             localTargetX * adjacent + localTargetY * opposite);

        const float desiredParent = (parentAngle - offset) * 180.0f / 3.14159265358979323846f;
        bonePose[static_cast<std::size_t>(parentIndex)].rotation += NormalizeAngle(desiredParent - bonePose[static_cast<std::size_t>(parentIndex)].rotation) * alpha;

        const float desiredChild = (childAngle + offset) * 180.0f / 3.14159265358979323846f;
        bonePose[static_cast<std::size_t>(childIndex)].rotation += NormalizeAngle(desiredChild - bonePose[static_cast<std::size_t>(childIndex)].rotation) * alpha;
    }

    std::vector<int> BuildCurrentDrawOrder() const
    {
        std::vector<int> order(m_slots.size());
        for (std::size_t i = 0; i < order.size(); ++i)
            order[i] = static_cast<int>(i);

        const auto animationIt = m_animations.find(m_currentAnimation);
        if (animationIt != m_animations.end())
        {
            const AnimationData& animation = animationIt->second;
            const float time = NormalizeTimelineTime(m_currentTime, animation.duration, m_currentLoop);
            if (!animation.drawOrder.empty())
                order = SampleDrawOrderFrame(animation.drawOrder, time, m_slots.size());
        }

        for (const TrackState& track : m_additionalTracks)
        {
            const auto trackAnimationIt = m_animations.find(track.animation);
            if (trackAnimationIt == m_animations.end())
                continue;
            const AnimationData& animation = trackAnimationIt->second;
            if (animation.drawOrder.empty())
                continue;
            const float time = NormalizeTimelineTime(track.time, animation.duration, track.loop);
            order = SampleDrawOrderFrame(animation.drawOrder, time, m_slots.size());
        }

        return order;
    }

    void LoadBones(const std::string& jsonText)
    {
        std::size_t bonesBegin = 0;
        std::size_t bonesEnd = 0;
        if (!FindTopLevelBlock(jsonText, "bones", '[', ']', bonesBegin, bonesEnd))
            return;

        ForEachObjectInArray(jsonText, bonesBegin, bonesEnd, [this, &jsonText](std::size_t begin, std::size_t end)
        {
            BoneData bone{};
            if (!ReadStringPropertyInObject(jsonText, begin, end, "name", bone.name))
                return;

            ReadStringPropertyInObject(jsonText, begin, end, "parent", bone.parentName);
            ReadFloatPropertyInObject(jsonText, begin, end, "x", bone.x);
            ReadFloatPropertyInObject(jsonText, begin, end, "y", bone.y);
            ReadFloatPropertyInObject(jsonText, begin, end, "rotation", bone.rotation);
            ReadFloatPropertyInObject(jsonText, begin, end, "scaleX", bone.scaleX);
            ReadFloatPropertyInObject(jsonText, begin, end, "scaleY", bone.scaleY);
            ReadFloatPropertyInObject(jsonText, begin, end, "length", bone.length);
            m_bones.push_back(std::move(bone));
        });
    }

    void LoadSlots(const std::string& jsonText)
    {
        std::size_t slotsBegin = 0;
        std::size_t slotsEnd = 0;
        if (!FindTopLevelBlock(jsonText, "slots", '[', ']', slotsBegin, slotsEnd))
            return;

        ForEachObjectInArray(jsonText, slotsBegin, slotsEnd, [this, &jsonText](std::size_t begin, std::size_t end)
        {
            SlotData slot{};
            if (!ReadStringPropertyInObject(jsonText, begin, end, "name", slot.name))
                return;

            ReadStringPropertyInObject(jsonText, begin, end, "bone", slot.boneName);
            ReadStringPropertyInObject(jsonText, begin, end, "attachment", slot.attachment);

            std::string colorValue;
            if (ReadStringPropertyInObject(jsonText, begin, end, "color", colorValue))
                slot.color = ParseHexColor(colorValue);

            std::string blendValue;
            if (ReadStringPropertyInObject(jsonText, begin, end, "blend", blendValue))
            {
                if (blendValue == "additive")
                    slot.blend = BlendMode::Additive;
                else if (blendValue == "multiply")
                    slot.blend = BlendMode::Multiply;
                else if (blendValue == "screen")
                    slot.blend = BlendMode::Screen;
            }

            m_slots.push_back(std::move(slot));
        });
    }

    bool LoadBinarySkeleton(const LoadRequest& request)
    {
        std::string data;
        if (!LoadSkeletonBinaryData(request, data))
            return false;

        BinaryReader reader(data);
        reader.String();
        reader.String();
        reader.Float();
        reader.Float();

        const bool nonessential = reader.Bool();
        if (nonessential)
            reader.String();

        const int boneCount = reader.VarInt(true);
        if (boneCount <= 0)
        {
            m_lastError = "Spine 2.1 binary load failed: no bones.";
            return false;
        }

        m_bones.reserve(static_cast<std::size_t>(boneCount));
        for (int i = 0; i < boneCount; ++i)
        {
            BoneData bone{};
            bone.name = reader.String();
            const int parentIndex = reader.VarInt(true) - 1;
            if (parentIndex >= 0 && parentIndex < static_cast<int>(m_bones.size()))
                bone.parentName = m_bones[static_cast<std::size_t>(parentIndex)].name;
            bone.parent = parentIndex;
            bone.x = reader.Float();
            bone.y = reader.Float();
            bone.scaleX = reader.Float();
            bone.scaleY = reader.Float();
            bone.rotation = reader.Float();
            bone.length = reader.Float();
            reader.Bool();
            reader.Bool();
            bone.inheritScale = reader.Bool();
            bone.inheritRotation = reader.Bool();
            if (nonessential)
                reader.Int32();
            m_bones.push_back(std::move(bone));
        }

        const int ikCount = reader.VarInt(true);
        for (int i = 0; i < ikCount; ++i)
        {
            IkConstraintData ik{};
            ik.name = reader.String();
            const int ikBoneCount = reader.VarInt(true);
            for (int b = 0; b < ikBoneCount; ++b)
            {
                const int boneIndex = reader.VarInt(true);
                if (boneIndex >= 0 && boneIndex < static_cast<int>(m_bones.size()))
                {
                    ik.bones.push_back(boneIndex);
                    ik.boneNames.push_back(m_bones[static_cast<std::size_t>(boneIndex)].name);
                }
            }
            ik.target = reader.VarInt(true);
            if (ik.target >= 0 && ik.target < static_cast<int>(m_bones.size()))
                ik.targetName = m_bones[static_cast<std::size_t>(ik.target)].name;
            ik.mix = reader.Float();
            ik.bendDirection = reader.S8();
            m_ikConstraints.push_back(std::move(ik));
        }

        const int slotCount = reader.VarInt(true);
        m_slots.reserve(static_cast<std::size_t>(std::max(0, slotCount)));
        for (int i = 0; i < slotCount; ++i)
        {
            SlotData slot{};
            slot.name = reader.String();
            const int boneIndex = reader.VarInt(true);
            slot.bone = boneIndex;
            if (boneIndex >= 0 && boneIndex < static_cast<int>(m_bones.size()))
                slot.boneName = m_bones[static_cast<std::size_t>(boneIndex)].name;
            slot.color = DecodePackedColor(reader.Int32());
            slot.attachment = reader.String();
            slot.blend = reader.Bool() ? BlendMode::Additive : BlendMode::Normal;
            m_slotNames.push_back(slot.name);
            m_slots.push_back(std::move(slot));
        }

        ReadBinarySkin(reader, "default", nonessential);
        if (std::find(m_skinNames.begin(), m_skinNames.end(), "default") == m_skinNames.end())
            m_skinNames.push_back("default");

        const int namedSkinCount = reader.VarInt(true);
        for (int i = 0; i < namedSkinCount; ++i)
        {
            const std::string skinName = reader.String();
            m_skinNames.push_back(skinName);
            ReadBinarySkin(reader, skinName, nonessential);
        }

        const int eventCount = reader.VarInt(true);
        for (int i = 0; i < eventCount; ++i)
        {
            reader.String();
            reader.VarInt(false);
            reader.Float();
            reader.String();
        }

        const int animationCount = reader.VarInt(true);
        for (int i = 0; i < animationCount; ++i)
        {
            const std::string animationName = reader.String();
            AnimationData animation{};
            animation.name = animationName;
            ReadBinaryAnimation(reader, animation);
            m_animationNames.push_back(animationName);
            m_animations[animationName] = std::move(animation);
        }

        if (!reader.Ok())
        {
            m_lastError = "Spine 2.1 binary load failed: unexpected end of file.";
            return false;
        }
        return true;
    }

    void LoadIkConstraints(const std::string& jsonText)
    {
        std::size_t ikBegin = 0;
        std::size_t ikEnd = 0;
        if (!FindTopLevelBlock(jsonText, "ik", '[', ']', ikBegin, ikEnd))
            return;

        ForEachObjectInArray(jsonText, ikBegin, ikEnd, [this, &jsonText](std::size_t begin, std::size_t end)
        {
            IkConstraintData ik{};
            ReadStringPropertyInObject(jsonText, begin, end, "name", ik.name);
            ReadStringArrayPropertyInObject(jsonText, begin, end, "bones", ik.boneNames);
            ReadStringPropertyInObject(jsonText, begin, end, "target", ik.targetName);
            ReadFloatPropertyInObject(jsonText, begin, end, "mix", ik.mix);

            float bendPositive = 1.0f;
            if (ReadFloatPropertyInObject(jsonText, begin, end, "bendPositive", bendPositive))
                ik.bendDirection = bendPositive == 0.0f ? -1 : 1;

            m_ikConstraints.push_back(std::move(ik));
        });
    }

    void LoadSkins(const std::string& jsonText)
    {
        std::size_t skinsBegin = 0;
        std::size_t skinsEnd = 0;
        if (!FindTopLevelBlock(jsonText, "skins", '{', '}', skinsBegin, skinsEnd))
            return;

        ForEachObjectMember(jsonText, skinsBegin, skinsEnd, [this, &jsonText](const std::string& skinName, std::size_t skinBegin, std::size_t skinEnd)
        {
            ForEachObjectMember(jsonText, skinBegin, skinEnd, [this, &jsonText, &skinName](const std::string& slotName, std::size_t slotBegin, std::size_t slotEnd)
            {
                ForEachObjectMember(jsonText, slotBegin, slotEnd, [this, &jsonText, &skinName, &slotName](const std::string& attachmentName, std::size_t attachmentBegin, std::size_t attachmentEnd)
                {
                    RegionAttachment attachment{};
                    attachment.slotName = slotName;
                    attachment.name = attachmentName;
                    attachment.color = Color{};

                    std::string typeValue;
                    ReadStringPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "type", typeValue);
                    if (!typeValue.empty() && typeValue != "region" && typeValue != "mesh" && typeValue != "skinnedmesh")
                        return;
                    attachment.mesh = typeValue == "mesh";
                    attachment.skinnedMesh = typeValue == "skinnedmesh";

                    ReadStringPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "name", attachment.path);
                    ReadStringPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "path", attachment.path);
                    ReadFloatPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "x", attachment.x);
                    ReadFloatPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "y", attachment.y);
                    ReadFloatPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "scaleX", attachment.scaleX);
                    ReadFloatPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "scaleY", attachment.scaleY);
                    ReadFloatPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "rotation", attachment.rotation);
                    ReadFloatPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "width", attachment.width);
                    ReadFloatPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "height", attachment.height);

                    std::string colorValue;
                    if (ReadStringPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "color", colorValue))
                        attachment.color = ParseHexColor(colorValue);

                    if (attachment.mesh || attachment.skinnedMesh)
                    {
                        ReadFloatArrayPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "uvs", attachment.uvs);
                        ReadIndexArrayPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "triangles", attachment.indices);

                        std::vector<float> rawVertices;
                        ReadFloatArrayPropertyInObject(jsonText, attachmentBegin, attachmentEnd, "vertices", rawVertices);
                        if (attachment.skinnedMesh)
                        {
                            ParseWeightedVertices(rawVertices, attachment.uvs.size() / 2, attachment.weightedVertices);
                            if (attachment.weightedVertices.empty() || attachment.uvs.empty() || attachment.indices.empty())
                                return;
                        }
                        else
                        {
                            attachment.vertices = std::move(rawVertices);
                        }

                        if (!attachment.skinnedMesh && (attachment.vertices.empty() || attachment.uvs.empty() || attachment.indices.empty()))
                            return;

                        UpdateMeshUvsFromAtlas(attachment);
                    }

                    m_attachments[SkinAttachmentKey(skinName, slotName, attachmentName)] = std::move(attachment);
                });
            });
        });
    }

    static void ParseWeightedVertices(const std::vector<float>& rawVertices, std::size_t vertexCount,
                                      std::vector<std::vector<RegionAttachment::WeightedBone>>& outVertices)
    {
        outVertices.clear();
        outVertices.reserve(vertexCount);

        std::size_t pos = 0;
        for (std::size_t vertexIndex = 0; vertexIndex < vertexCount && pos < rawVertices.size(); ++vertexIndex)
        {
            const int boneCount = static_cast<int>(rawVertices[pos++]);
            std::vector<RegionAttachment::WeightedBone> weights;
            weights.reserve(static_cast<std::size_t>(std::max(0, boneCount)));

            for (int i = 0; i < boneCount && pos + 3 < rawVertices.size(); ++i)
            {
                RegionAttachment::WeightedBone weight{};
                weight.bone = static_cast<int>(rawVertices[pos++]);
                weight.x = rawVertices[pos++];
                weight.y = rawVertices[pos++];
                weight.weight = rawVertices[pos++];
                weights.push_back(weight);
            }

            outVertices.push_back(std::move(weights));
        }
    }

    void UpdateMeshUvsFromAtlas(RegionAttachment& attachment) const
    {
        if (!attachment.mesh && !attachment.skinnedMesh)
            return;

        const std::string regionKey = attachment.path.empty() ? attachment.name : attachment.path;
        const AtlasRegion* region = FindAtlasRegion(regionKey);
        if (region == nullptr || attachment.uvs.empty())
            return;

        const float width = region->u2 - region->u;
        const float height = region->v2 - region->v;
        std::vector<float> finalUvs(attachment.uvs.size(), 0.0f);
        for (std::size_t i = 0; i + 1 < attachment.uvs.size(); i += 2)
        {
            const float sourceU = attachment.uvs[i];
            const float sourceV = attachment.uvs[i + 1];
            if (region->rotate)
            {
                finalUvs[i] = region->u + sourceV * width;
                finalUvs[i + 1] = region->v2 - sourceU * height;
            }
            else
            {
                finalUvs[i] = region->u + sourceU * width;
                finalUvs[i + 1] = region->v + sourceV * height;
            }
        }
        attachment.uvs = std::move(finalUvs);
    }

    void ReadBinarySkin(BinaryReader& reader, const std::string& skinName, bool nonessential)
    {
        const int slotAttachmentGroups = reader.VarInt(true);
        for (int i = 0; i < slotAttachmentGroups; ++i)
        {
            const int slotIndex = reader.VarInt(true);
            const int attachmentCount = reader.VarInt(true);
            for (int j = 0; j < attachmentCount; ++j)
            {
                const std::string entryName = reader.String();
                RegionAttachment attachment;
                const bool keep = ReadBinaryAttachment(reader, slotIndex, entryName, nonessential, attachment);
                if (keep)
                    UpdateMeshUvsFromAtlas(attachment);
                if (keep && slotIndex >= 0 && slotIndex < static_cast<int>(m_slots.size()))
                    m_attachments[SkinAttachmentKey(skinName, m_slots[static_cast<std::size_t>(slotIndex)].name, entryName)] = std::move(attachment);
            }
        }
    }

    bool ReadBinaryAttachment(BinaryReader& reader, int slotIndex, const std::string& defaultName, bool nonessential,
                              RegionAttachment& attachment)
    {
        std::string name = reader.String();
        if (name.empty())
            name = defaultName;

        const int type = reader.U8();
        if (slotIndex >= 0 && slotIndex < static_cast<int>(m_slots.size()))
            attachment.slotName = m_slots[static_cast<std::size_t>(slotIndex)].name;
        attachment.name = name;

        switch (type)
        {
        case 0:
        {
            attachment.path = reader.String();
            if (attachment.path.empty())
                attachment.path = name;
            attachment.x = reader.Float();
            attachment.y = reader.Float();
            attachment.scaleX = reader.Float();
            attachment.scaleY = reader.Float();
            attachment.rotation = reader.Float();
            attachment.width = reader.Float();
            attachment.height = reader.Float();
            attachment.color = DecodePackedColor(reader.Int32());
            return true;
        }
        case 1:
            reader.FloatArray(1.0f);
            return false;
        case 2:
        {
            attachment.mesh = true;
            attachment.path = reader.String();
            if (attachment.path.empty())
                attachment.path = name;
            attachment.uvs = reader.FloatArray(1.0f);
            attachment.indices = reader.ShortArray();
            attachment.vertices = reader.FloatArray(1.0f);
            attachment.color = DecodePackedColor(reader.Int32());
            reader.VarInt(true);
            if (nonessential)
            {
                reader.SkipIntArray();
                attachment.width = reader.Float();
                attachment.height = reader.Float();
            }
            return !attachment.uvs.empty() && !attachment.indices.empty() && !attachment.vertices.empty();
        }
        case 3:
        {
            attachment.skinnedMesh = true;
            attachment.path = reader.String();
            if (attachment.path.empty())
                attachment.path = name;
            attachment.uvs = reader.FloatArray(1.0f);
            attachment.indices = reader.ShortArray();
            const int rawCount = reader.VarInt(true);
            std::vector<float> raw;
            raw.reserve(static_cast<std::size_t>(std::max(0, rawCount)));
            for (int i = 0; i < rawCount; ++i)
                raw.push_back(reader.Float());
            ParseWeightedVertices(raw, attachment.uvs.size() / 2, attachment.weightedVertices);
            attachment.color = DecodePackedColor(reader.Int32());
            reader.VarInt(true);
            if (nonessential)
            {
                reader.SkipIntArray();
                attachment.width = reader.Float();
                attachment.height = reader.Float();
            }
            return !attachment.weightedVertices.empty() && !attachment.uvs.empty() && !attachment.indices.empty();
        }
        default:
            return false;
        }
    }

    void ResolveBoneLinks()
    {
        std::unordered_map<std::string, int> boneByName;
        for (std::size_t i = 0; i < m_bones.size(); ++i)
            boneByName[m_bones[i].name] = static_cast<int>(i);

        for (BoneData& bone : m_bones)
        {
            if (!bone.parentName.empty())
            {
                const auto it = boneByName.find(bone.parentName);
                bone.parent = it == boneByName.end() ? -1 : it->second;
            }
        }

        for (SlotData& slot : m_slots)
        {
            const auto it = boneByName.find(slot.boneName);
            slot.bone = it == boneByName.end() ? -1 : it->second;
        }

        for (IkConstraintData& ik : m_ikConstraints)
        {
            ik.bones.clear();
            for (const std::string& boneName : ik.boneNames)
            {
                const auto it = boneByName.find(boneName);
                if (it != boneByName.end())
                    ik.bones.push_back(it->second);
            }

            const auto targetIt = boneByName.find(ik.targetName);
            ik.target = targetIt == boneByName.end() ? -1 : targetIt->second;
        }
    }

    int FindBoneIndex(const std::string& name) const
    {
        for (std::size_t i = 0; i < m_bones.size(); ++i)
        {
            if (m_bones[i].name == name)
                return static_cast<int>(i);
        }
        return -1;
    }

    int FindSlotIndex(const std::string& name) const
    {
        for (std::size_t i = 0; i < m_slots.size(); ++i)
        {
            if (m_slots[i].name == name)
                return static_cast<int>(i);
        }
        return -1;
    }

    static void SortFloatFrames(std::vector<FloatFrame>& frames)
    {
        std::sort(frames.begin(), frames.end(), [](const FloatFrame& a, const FloatFrame& b) { return a.time < b.time; });
    }

    static void SortColorFrames(std::vector<ColorFrame>& frames)
    {
        std::sort(frames.begin(), frames.end(), [](const ColorFrame& a, const ColorFrame& b) { return a.time < b.time; });
    }

    static void SortAttachmentFrames(std::vector<AttachmentFrame>& frames)
    {
        std::sort(frames.begin(), frames.end(), [](const AttachmentFrame& a, const AttachmentFrame& b) { return a.time < b.time; });
    }

    static void SortDrawOrderFrames(std::vector<DrawOrderFrame>& frames)
    {
        std::sort(frames.begin(), frames.end(), [](const DrawOrderFrame& a, const DrawOrderFrame& b) { return a.time < b.time; });
    }

    static void SortFfdFrames(std::vector<FfdFrame>& frames)
    {
        std::sort(frames.begin(), frames.end(), [](const FfdFrame& a, const FfdFrame& b) { return a.time < b.time; });
    }

    void ParseFloatTimeline(const std::string& jsonText, std::size_t arrayBegin, std::size_t arrayEnd,
                            const std::string& firstKey, const std::string& secondKey,
                            float defaultFirst, float defaultSecond,
                            std::vector<FloatFrame>& outFrames, float& duration) const
    {
        ForEachObjectInArray(jsonText, arrayBegin, arrayEnd, [&jsonText, &firstKey, &secondKey, defaultFirst, defaultSecond, &outFrames, &duration](std::size_t begin, std::size_t end)
        {
            FloatFrame frame{};
            frame.a = defaultFirst;
            frame.b = defaultSecond;
            ReadFloatPropertyInObject(jsonText, begin, end, "time", frame.time);
            ReadFloatPropertyInObject(jsonText, begin, end, firstKey, frame.a);
            if (!secondKey.empty())
                ReadFloatPropertyInObject(jsonText, begin, end, secondKey, frame.b);
            ReadCurve(jsonText, begin, end, frame);
            duration = std::max(duration, frame.time);
            outFrames.push_back(frame);
        });
        SortFloatFrames(outFrames);
    }

    void ParseColorTimeline(const std::string& jsonText, std::size_t arrayBegin, std::size_t arrayEnd,
                            std::vector<ColorFrame>& outFrames, float& duration) const
    {
        ForEachObjectInArray(jsonText, arrayBegin, arrayEnd, [&jsonText, &outFrames, &duration](std::size_t begin, std::size_t end)
        {
            ColorFrame frame{};
            ReadFloatPropertyInObject(jsonText, begin, end, "time", frame.time);

            std::string colorValue;
            if (ReadStringPropertyInObject(jsonText, begin, end, "color", colorValue))
                frame.color = ParseHexColor(colorValue);

            ReadCurve(jsonText, begin, end, frame);
            duration = std::max(duration, frame.time);
            outFrames.push_back(frame);
        });
        SortColorFrames(outFrames);
    }

    void ParseAttachmentTimeline(const std::string& jsonText, std::size_t arrayBegin, std::size_t arrayEnd,
                                 std::vector<AttachmentFrame>& outFrames, float& duration) const
    {
        ForEachObjectInArray(jsonText, arrayBegin, arrayEnd, [&jsonText, &outFrames, &duration](std::size_t begin, std::size_t end)
        {
            AttachmentFrame frame{};
            ReadFloatPropertyInObject(jsonText, begin, end, "time", frame.time);
            ReadStringPropertyInObject(jsonText, begin, end, "name", frame.name);
            duration = std::max(duration, frame.time);
            outFrames.push_back(std::move(frame));
        });
        SortAttachmentFrames(outFrames);
    }

    void ParseDrawOrderTimeline(const std::string& jsonText, std::size_t arrayBegin, std::size_t arrayEnd,
                                std::vector<DrawOrderFrame>& outFrames, float& duration) const
    {
        ForEachObjectInArray(jsonText, arrayBegin, arrayEnd, [this, &jsonText, &outFrames, &duration](std::size_t begin, std::size_t end)
        {
            DrawOrderFrame frame{};
            ReadFloatPropertyInObject(jsonText, begin, end, "time", frame.time);
            frame.order.resize(m_slots.size());
            std::vector<int> unchanged;
            unchanged.reserve(m_slots.size());
            for (std::size_t i = 0; i < m_slots.size(); ++i)
                frame.order[i] = -1;

            std::size_t offsetsPos = 0;
            if (FindObjectMemberValue(jsonText, begin, end, "offsets", offsetsPos) && jsonText[offsetsPos] == '[')
            {
                std::size_t offsetsEnd = 0;
                if (FindMatchingBracket(jsonText, offsetsPos, '[', ']', offsetsEnd))
                {
                    int originalIndex = 0;
                    ForEachObjectInArray(jsonText, offsetsPos, offsetsEnd, [this, &jsonText, &frame, &unchanged, &originalIndex](std::size_t offsetBegin, std::size_t offsetEnd)
                    {
                        std::string slotName;
                        if (!ReadStringPropertyInObject(jsonText, offsetBegin, offsetEnd, "slot", slotName))
                            return;

                        const int slotIndex = FindSlotIndex(slotName);
                        if (slotIndex < 0)
                            return;

                        while (originalIndex < slotIndex && originalIndex < static_cast<int>(m_slots.size()))
                            unchanged.push_back(originalIndex++);

                        float offsetValue = 0.0f;
                        ReadFloatPropertyInObject(jsonText, offsetBegin, offsetEnd, "offset", offsetValue);
                        const int targetIndex = originalIndex + static_cast<int>(offsetValue);
                        if (targetIndex >= 0 && targetIndex < static_cast<int>(frame.order.size()))
                            frame.order[static_cast<std::size_t>(targetIndex)] = originalIndex;
                        ++originalIndex;
                    });

                    while (originalIndex < static_cast<int>(m_slots.size()))
                        unchanged.push_back(originalIndex++);

                    for (int i = static_cast<int>(frame.order.size()) - 1; i >= 0; --i)
                    {
                        if (frame.order[static_cast<std::size_t>(i)] == -1 && !unchanged.empty())
                        {
                            frame.order[static_cast<std::size_t>(i)] = unchanged.back();
                            unchanged.pop_back();
                        }
                    }
                }
            }
            else
            {
                for (std::size_t i = 0; i < frame.order.size(); ++i)
                    frame.order[i] = static_cast<int>(i);
            }

            duration = std::max(duration, frame.time);
            outFrames.push_back(std::move(frame));
        });
        SortDrawOrderFrames(outFrames);
    }

    template <typename TFrame>
    static void ReadBinaryCurve(BinaryReader& reader, TFrame& frame)
    {
        const int curveType = reader.U8();
        if (curveType == 1)
        {
            frame.curve = CurveMode::Stepped;
        }
        else if (curveType == 2)
        {
            frame.curve = CurveMode::Bezier;
            frame.cx1 = reader.Float();
            frame.cy1 = reader.Float();
            frame.cx2 = reader.Float();
            frame.cy2 = reader.Float();
        }
    }

    void ReadBinarySlotTimelines(BinaryReader& reader, AnimationData& animation)
    {
        const int slotTimelineCount = reader.VarInt(true);
        for (int i = 0; i < slotTimelineCount; ++i)
        {
            const int slotIndex = reader.VarInt(true);
            const int timelineCount = reader.VarInt(true);
            SlotTimeline& timeline = animation.slots[slotIndex];
            for (int t = 0; t < timelineCount; ++t)
            {
                const int type = reader.U8();
                const int frameCount = reader.VarInt(true);
                if (type == 4)
                {
                    for (int f = 0; f < frameCount; ++f)
                    {
                        ColorFrame frame{};
                        frame.time = reader.Float();
                        frame.color = DecodePackedColor(reader.Int32());
                        if (f < frameCount - 1)
                            ReadBinaryCurve(reader, frame);
                        animation.duration = std::max(animation.duration, frame.time);
                        timeline.color.push_back(frame);
                    }
                    SortColorFrames(timeline.color);
                }
                else if (type == 3)
                {
                    for (int f = 0; f < frameCount; ++f)
                    {
                        AttachmentFrame frame{};
                        frame.time = reader.Float();
                        frame.name = reader.String();
                        animation.duration = std::max(animation.duration, frame.time);
                        timeline.attachment.push_back(std::move(frame));
                    }
                    SortAttachmentFrames(timeline.attachment);
                }
                else
                {
                    SkipUnknownTimeline(reader, frameCount, false);
                }
            }
        }
    }

    void ReadBinaryBoneTimelines(BinaryReader& reader, AnimationData& animation)
    {
        const int boneTimelineCount = reader.VarInt(true);
        for (int i = 0; i < boneTimelineCount; ++i)
        {
            const int boneIndex = reader.VarInt(true);
            const int timelineCount = reader.VarInt(true);
            BoneTimeline& timeline = animation.bones[boneIndex];
            for (int t = 0; t < timelineCount; ++t)
            {
                const int type = reader.U8();
                const int frameCount = reader.VarInt(true);
                if (type == 1)
                {
                    for (int f = 0; f < frameCount; ++f)
                    {
                        FloatFrame frame{};
                        frame.time = reader.Float();
                        frame.a = reader.Float();
                        if (f < frameCount - 1)
                            ReadBinaryCurve(reader, frame);
                        animation.duration = std::max(animation.duration, frame.time);
                        timeline.rotate.push_back(frame);
                    }
                    SortFloatFrames(timeline.rotate);
                }
                else if (type == 2 || type == 0)
                {
                    std::vector<FloatFrame>& frames = type == 2 ? timeline.translate : timeline.scale;
                    for (int f = 0; f < frameCount; ++f)
                    {
                        FloatFrame frame{};
                        frame.time = reader.Float();
                        frame.a = reader.Float();
                        frame.b = reader.Float();
                        if (f < frameCount - 1)
                            ReadBinaryCurve(reader, frame);
                        animation.duration = std::max(animation.duration, frame.time);
                        frames.push_back(frame);
                    }
                    SortFloatFrames(frames);
                }
                else if (type == 5 || type == 6)
                {
                    for (int f = 0; f < frameCount; ++f)
                    {
                        animation.duration = std::max(animation.duration, reader.Float());
                        reader.Bool();
                    }
                }
                else
                {
                    SkipUnknownTimeline(reader, frameCount, true);
                }
            }
        }
    }

    void ReadBinaryIkTimelines(BinaryReader& reader, AnimationData& animation)
    {
        const int ikTimelineCount = reader.VarInt(true);
        for (int i = 0; i < ikTimelineCount; ++i)
        {
            reader.VarInt(true);
            const int frameCount = reader.VarInt(true);
            for (int f = 0; f < frameCount; ++f)
            {
                animation.duration = std::max(animation.duration, reader.Float());
                reader.Float();
                reader.S8();
                if (f < frameCount - 1)
                    SkipBinaryCurve(reader);
            }
        }
    }

    void ReadBinaryFfdTimelines(BinaryReader& reader, AnimationData& animation)
    {
        const int skinCount = reader.VarInt(true);
        for (int i = 0; i < skinCount; ++i)
        {
            reader.VarInt(true);
            const int slotGroupCount = reader.VarInt(true);
            for (int s = 0; s < slotGroupCount; ++s)
            {
                const int slotIndex = reader.VarInt(true);
                const int attachmentCount = reader.VarInt(true);
                for (int a = 0; a < attachmentCount; ++a)
                {
                    const std::string attachmentName = reader.String();
                    const int frameCount = reader.VarInt(true);
                    FfdTimeline timeline{};
                    timeline.slot = slotIndex;
                    timeline.attachmentName = attachmentName;
                    const RegionAttachment* attachment = slotIndex >= 0 && static_cast<std::size_t>(slotIndex) < m_slots.size()
                        ? FindAttachment(m_slots[static_cast<std::size_t>(slotIndex)].name, attachmentName)
                        : nullptr;
                    std::size_t vertexCount = 0;
                    if (attachment != nullptr && attachment->skinnedMesh)
                    {
                        for (const auto& weightedVertex : attachment->weightedVertices)
                            vertexCount += weightedVertex.size() * 2;
                    }
                    else if (attachment != nullptr)
                    {
                        vertexCount = attachment->vertices.size();
                    }
                    for (int f = 0; f < frameCount; ++f)
                    {
                        FfdFrame frame{};
                        frame.time = reader.Float();
                        animation.duration = std::max(animation.duration, frame.time);
                        const int end = reader.VarInt(true);
                        if (end != 0)
                        {
                            const int start = reader.VarInt(true);
                            frame.vertices.assign(vertexCount, 0.0f);
                            for (int v = 0; v < end; ++v)
                            {
                                const std::size_t index = static_cast<std::size_t>(start + v);
                                const float value = reader.Float();
                                if (index < frame.vertices.size())
                                    frame.vertices[index] = value;
                            }
                            if (attachment != nullptr && !attachment->skinnedMesh)
                            {
                                const std::size_t count = std::min(frame.vertices.size(), attachment->vertices.size());
                                for (std::size_t v = 0; v < count; ++v)
                                    frame.vertices[v] += attachment->vertices[v];
                            }
                        }
                        else
                        {
                            if (attachment != nullptr && !attachment->skinnedMesh)
                                frame.vertices = attachment->vertices;
                            else
                                frame.vertices.assign(vertexCount, 0.0f);
                        }
                        if (f < frameCount - 1)
                            ReadBinaryCurve(reader, frame);
                        timeline.frames.push_back(std::move(frame));
                    }
                    SortFfdFrames(timeline.frames);
                    if (!timeline.frames.empty() && timeline.slot >= 0)
                        animation.ffd.push_back(std::move(timeline));
                }
            }
        }
    }

    void ReadBinaryDrawOrderTimeline(BinaryReader& reader, AnimationData& animation)
    {
        const int frameCount = reader.VarInt(true);
        for (int f = 0; f < frameCount; ++f)
        {
            DrawOrderFrame frame{};
            frame.order.assign(m_slots.size(), -1);
            std::vector<int> unchanged;
            const int offsetCount = reader.VarInt(true);
            int originalIndex = 0;
            for (int i = 0; i < offsetCount; ++i)
            {
                const int slotIndex = reader.VarInt(true);
                while (originalIndex != slotIndex && originalIndex < static_cast<int>(m_slots.size()))
                    unchanged.push_back(originalIndex++);

                const int targetIndex = originalIndex + reader.VarInt(true);
                if (targetIndex >= 0 && targetIndex < static_cast<int>(frame.order.size()))
                    frame.order[static_cast<std::size_t>(targetIndex)] = originalIndex;
                ++originalIndex;
            }

            while (originalIndex < static_cast<int>(m_slots.size()))
                unchanged.push_back(originalIndex++);

            for (int i = static_cast<int>(frame.order.size()) - 1; i >= 0; --i)
            {
                if (frame.order[static_cast<std::size_t>(i)] == -1 && !unchanged.empty())
                {
                    frame.order[static_cast<std::size_t>(i)] = unchanged.back();
                    unchanged.pop_back();
                }
            }

            frame.time = reader.Float();
            animation.duration = std::max(animation.duration, frame.time);
            animation.drawOrder.push_back(std::move(frame));
        }
        SortDrawOrderFrames(animation.drawOrder);
    }

    void ReadBinaryEventTimeline(BinaryReader& reader, AnimationData& animation)
    {
        const int frameCount = reader.VarInt(true);
        for (int i = 0; i < frameCount; ++i)
        {
            animation.duration = std::max(animation.duration, reader.Float());
            reader.VarInt(true);
            reader.VarInt(false);
            reader.Float();
            if (reader.Bool())
                reader.String();
        }
    }

    void ReadBinaryAnimation(BinaryReader& reader, AnimationData& animation)
    {
        ReadBinarySlotTimelines(reader, animation);
        ReadBinaryBoneTimelines(reader, animation);
        ReadBinaryIkTimelines(reader, animation);
        ReadBinaryFfdTimelines(reader, animation);
        ReadBinaryDrawOrderTimeline(reader, animation);
        ReadBinaryEventTimeline(reader, animation);
    }

    static void SkipBinaryCurve(BinaryReader& reader)
    {
        const int curve = reader.U8();
        if (curve == 2)
        {
            reader.Float();
            reader.Float();
            reader.Float();
            reader.Float();
        }
    }

    static void SkipUnknownTimeline(BinaryReader& reader, int frameCount, bool boneTimeline)
    {
        for (int i = 0; i < frameCount; ++i)
        {
            reader.Float();
            if (boneTimeline)
            {
                reader.Float();
                reader.Float();
            }
            else
            {
                reader.String();
            }
            if (i < frameCount - 1)
                SkipBinaryCurve(reader);
        }
    }

    void LoadBoneAnimation(const std::string& jsonText, const std::string& boneName, std::size_t boneAnimBegin,
                           std::size_t boneAnimEnd, AnimationData& animation)
    {
        const int boneIndex = FindBoneIndex(boneName);
        if (boneIndex < 0)
            return;

        BoneTimeline timeline{};
        std::size_t timelinePos = 0;
        if (FindObjectMemberValue(jsonText, boneAnimBegin, boneAnimEnd, "rotate", timelinePos) && jsonText[timelinePos] == '[')
        {
            std::size_t timelineEnd = 0;
            if (FindMatchingBracket(jsonText, timelinePos, '[', ']', timelineEnd))
                ParseFloatTimeline(jsonText, timelinePos, timelineEnd, "angle", std::string(), 0.0f, 0.0f, timeline.rotate, animation.duration);
        }

        if (FindObjectMemberValue(jsonText, boneAnimBegin, boneAnimEnd, "translate", timelinePos) && jsonText[timelinePos] == '[')
        {
            std::size_t timelineEnd = 0;
            if (FindMatchingBracket(jsonText, timelinePos, '[', ']', timelineEnd))
                ParseFloatTimeline(jsonText, timelinePos, timelineEnd, "x", "y", 0.0f, 0.0f, timeline.translate, animation.duration);
        }

        if (FindObjectMemberValue(jsonText, boneAnimBegin, boneAnimEnd, "scale", timelinePos) && jsonText[timelinePos] == '[')
        {
            std::size_t timelineEnd = 0;
            if (FindMatchingBracket(jsonText, timelinePos, '[', ']', timelineEnd))
                ParseFloatTimeline(jsonText, timelinePos, timelineEnd, "x", "y", 1.0f, 1.0f, timeline.scale, animation.duration);
        }

        if (!timeline.rotate.empty() || !timeline.translate.empty() || !timeline.scale.empty())
            animation.bones[boneIndex] = std::move(timeline);
    }

    void LoadSlotAnimation(const std::string& jsonText, const std::string& slotName, std::size_t slotAnimBegin,
                           std::size_t slotAnimEnd, AnimationData& animation)
    {
        const int slotIndex = FindSlotIndex(slotName);
        if (slotIndex < 0)
            return;

        SlotTimeline timeline{};
        std::size_t timelinePos = 0;
        if (FindObjectMemberValue(jsonText, slotAnimBegin, slotAnimEnd, "color", timelinePos) && jsonText[timelinePos] == '[')
        {
            std::size_t timelineEnd = 0;
            if (FindMatchingBracket(jsonText, timelinePos, '[', ']', timelineEnd))
                ParseColorTimeline(jsonText, timelinePos, timelineEnd, timeline.color, animation.duration);
        }

        if (FindObjectMemberValue(jsonText, slotAnimBegin, slotAnimEnd, "attachment", timelinePos) && jsonText[timelinePos] == '[')
        {
            std::size_t timelineEnd = 0;
            if (FindMatchingBracket(jsonText, timelinePos, '[', ']', timelineEnd))
                ParseAttachmentTimeline(jsonText, timelinePos, timelineEnd, timeline.attachment, animation.duration);
        }

        if (!timeline.color.empty() || !timeline.attachment.empty())
            animation.slots[slotIndex] = std::move(timeline);
    }

    void LoadAnimations(const std::string& jsonText)
    {
        std::size_t animationsBegin = 0;
        std::size_t animationsEnd = 0;
        if (!FindTopLevelBlock(jsonText, "animations", '{', '}', animationsBegin, animationsEnd))
            return;

        ForEachObjectMember(jsonText, animationsBegin, animationsEnd, [this, &jsonText](const std::string& animationName, std::size_t animBegin, std::size_t animEnd)
        {
            AnimationData animation{};
            animation.name = animationName;

            std::size_t bonesPos = 0;
            if (FindObjectMemberValue(jsonText, animBegin, animEnd, "bones", bonesPos) && jsonText[bonesPos] == '{')
            {
                std::size_t bonesEnd = 0;
                if (FindMatchingBracket(jsonText, bonesPos, '{', '}', bonesEnd))
                {
                    ForEachObjectMember(jsonText, bonesPos, bonesEnd, [this, &jsonText, &animation](const std::string& boneName, std::size_t begin, std::size_t end)
                    {
                        LoadBoneAnimation(jsonText, boneName, begin, end, animation);
                    });
                }
            }

            std::size_t slotsPos = 0;
            if (FindObjectMemberValue(jsonText, animBegin, animEnd, "slots", slotsPos) && jsonText[slotsPos] == '{')
            {
                std::size_t slotsEnd = 0;
                if (FindMatchingBracket(jsonText, slotsPos, '{', '}', slotsEnd))
                {
                    ForEachObjectMember(jsonText, slotsPos, slotsEnd, [this, &jsonText, &animation](const std::string& slotName, std::size_t begin, std::size_t end)
                    {
                        LoadSlotAnimation(jsonText, slotName, begin, end, animation);
                    });
                }
            }

            std::size_t drawOrderPos = 0;
            if ((!FindObjectMemberValue(jsonText, animBegin, animEnd, "draworder", drawOrderPos)) &&
                (!FindObjectMemberValue(jsonText, animBegin, animEnd, "drawOrder", drawOrderPos)))
            {
                drawOrderPos = 0;
            }
            if (drawOrderPos != 0 && jsonText[drawOrderPos] == '[')
            {
                std::size_t drawOrderEnd = 0;
                if (FindMatchingBracket(jsonText, drawOrderPos, '[', ']', drawOrderEnd))
                    ParseDrawOrderTimeline(jsonText, drawOrderPos, drawOrderEnd, animation.drawOrder, animation.duration);
            }

            m_animations[animationName] = std::move(animation);
        });
    }

    const RegionAttachment* FindAttachment(const std::string& slotName, const std::string& attachmentName) const
    {
        const std::string skin = m_currentSkin.empty() ? "default" : m_currentSkin;
        auto it = m_attachments.find(SkinAttachmentKey(skin, slotName, attachmentName));
        if (it != m_attachments.end())
            return &it->second;

        if (skin != "default")
        {
            it = m_attachments.find(SkinAttachmentKey("default", slotName, attachmentName));
            if (it != m_attachments.end())
                return &it->second;
        }

        it = m_attachments.find(slotName + "\n" + attachmentName);
        return it == m_attachments.end() ? nullptr : &it->second;
    }

    static std::string SkinAttachmentKey(const std::string& skinName, const std::string& slotName, const std::string& attachmentName)
    {
        return skinName + "\n" + slotName + "\n" + attachmentName;
    }

    const AtlasRegion* FindAtlasRegion(const std::string& name) const
    {
        auto it = m_atlasRegions.find(name);
        if (it != m_atlasRegions.end())
            return &it->second;

        const std::size_t slash = name.find_last_of("\\/");
        if (slash != std::string::npos)
        {
            it = m_atlasRegions.find(name.substr(slash + 1));
            if (it != m_atlasRegions.end())
                return &it->second;
        }

        return nullptr;
    }

    std::vector<std::string> m_animationNames;
    std::vector<std::string> m_skinNames;
    std::vector<std::string> m_slotNames;
    std::vector<TextureInfo> m_textureInfos;
    std::vector<BoneData> m_bones;
    std::vector<SlotData> m_slots;
    std::vector<IkConstraintData> m_ikConstraints;
    std::map<std::string, AtlasRegion> m_atlasRegions;
    std::map<std::string, RegionAttachment> m_attachments;
    std::map<std::string, AnimationData> m_animations;
    std::string m_currentAnimation;
    std::string m_currentSkin;
    std::vector<TrackState> m_additionalTracks;
    float m_currentTime = 0.0f;
    bool m_currentLoop = true;
    std::string m_lastError;
    bool m_hasSkeleton = false;
};

}

std::unique_ptr<IRuntime> CreateCpp21Runtime()
{
    return std::make_unique<Spine21CppRuntime>();
}

}
