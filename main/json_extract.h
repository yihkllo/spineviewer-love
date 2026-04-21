#ifndef SL_JSON_PARSER_H_
#define SL_JSON_PARSER_H_

namespace json_minimal
{
	bool ExtractJsonObject(char** src, const char* name, char** dst);
	bool ExtractJsonArray(char** src, const char* name, char** dst);
	bool GetJsonElementValue(char* src, const char* name, char* dst, size_t nDstSize, int* iDepth = nullptr, char** pEnd = nullptr);

	bool ExtractArrayValueByIndices(char* src, const size_t* indices, size_t indices_size, char** dst);

	bool ReadNextKey(char** src, char* key, size_t nKeySize, char* value, size_t nValueSize);
	bool ReadNextArrayValue(char** src, char* dst, size_t nDstSize);

	bool ReadUpToNameEnd(char** src, const char* name = nullptr, char* value = nullptr, size_t nValueSize = 0);
}

#endif
