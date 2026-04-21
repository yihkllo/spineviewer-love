#ifndef SPINELOVE_DXLIB_SHARED_DLL_BRIDGE_H_
#define SPINELOVE_DXLIB_SHARED_DLL_BRIDGE_H_

#include <DxLib.h>

#if defined(_WIN32)
#if defined(SPINE_RUNTIME_DLL_BUILD)
#define SPINE_EXTERN __declspec(dllexport)
#else
#define SPINE_EXTERN __declspec(dllimport)
#endif
#else
#define SPINE_EXTERN
#endif

struct DxLibTextApi
{
#if defined(_WIN32) && defined(_UNICODE)
	int (*GetUseCharCodeFormat)(void) = nullptr;
	int (*Get_wchar_t_CharCodeFormat)(void) = nullptr;
	int (*ConvertStringCharCodeFormat)(int SrcCharCodeFormat, const void* SrcString, int DestCharCodeFormat, void* DestStringBuffer) = nullptr;

	bool AllValid() const noexcept
	{
		return GetUseCharCodeFormat && Get_wchar_t_CharCodeFormat && ConvertStringCharCodeFormat;
	}
#else
	bool AllValid() const noexcept
	{
		return true;
	}
#endif
};

struct DxLibGraphApi
{
	int (*LoadGraph)(const TCHAR* FileName, int NotUse3DFlag) = nullptr;
	int (*DeleteGraph)(int GrHandle) = nullptr;
	int (*SetUsePremulAlphaConvertLoad)(int UseFlag) = nullptr;
	int (*GetUsePremulAlphaConvertLoad)(void) = nullptr;
	int (*SetDrawCustomBlendMode)(int BlendEnable, int SrcBlendRGB, int DestBlendRGB, int BlendOpRGB, int SrcBlendA, int DestBlendA, int BlendOpA, int BlendParam) = nullptr;
	int (*SetDrawBlendMode)(int BlendMode, int BlendParam) = nullptr;
	int (*DrawPolygonIndexed2D)(const DxLib::VERTEX2D* VertexArray, int VertexNum, const unsigned short* IndexArray, int PolygonNum, int GrHandle, int TransFlag) = nullptr;

	bool AllValid() const noexcept
	{
		return LoadGraph && DeleteGraph
			&& SetUsePremulAlphaConvertLoad && GetUsePremulAlphaConvertLoad
			&& SetDrawCustomBlendMode && SetDrawBlendMode && DrawPolygonIndexed2D;
	}
};

struct DxLibTransformApi
{
	int (*GetDrawScreenSize)(int* XBuf, int* YBuf) = nullptr;
	DxLib::MATRIX (*MGetScale)(DxLib::VECTOR Scale) = nullptr;
	DxLib::MATRIX (*MGetTranslate)(DxLib::VECTOR Trans) = nullptr;
	DxLib::MATRIX (*MGetRotZ)(float angle) = nullptr;
	DxLib::MATRIX (*MMult)(DxLib::MATRIX In1, DxLib::MATRIX In2) = nullptr;
	int (*SetTransformTo2D)(const DxLib::MATRIX* Matrix) = nullptr;
	int (*ResetTransformTo2D)(void) = nullptr;

	bool AllValid() const noexcept
	{
		return GetDrawScreenSize && MGetScale && MGetTranslate && MGetRotZ && MMult
			&& SetTransformTo2D && ResetTransformTo2D;
	}
};

struct DxLibDisplayApi
{
#if defined _WIN32
	int (*GetDisplayMaxResolution)(int* SizeX, int* SizeY, int DisplayIndex) = nullptr;
#elif defined __ANDROID__
	int (*GetAndroidDisplayResolution)(int* SizeX, int* SizeY) = nullptr;
#elif defined __APPLE__
	int (*GetDisplayResolution_iOS)(int* SizeX, int* SizeY) = nullptr;
#endif

	bool AllValid() const noexcept
	{
#if defined _WIN32
		return GetDisplayMaxResolution != nullptr;
#elif defined __ANDROID__
		return GetAndroidDisplayResolution != nullptr;
#elif defined __APPLE__
		return GetDisplayResolution_iOS != nullptr;
#else
		return true;
#endif
	}
};

struct DxLibRegerenda
{
	DxLibTextApi text;
	DxLibGraphApi graph;
	DxLibTransformApi transform;
	DxLibDisplayApi display;

	bool AllValid() const noexcept
	{
		return text.AllValid() && graph.AllValid() && transform.AllValid() && display.AllValid();
	}
};

extern "C"
{
	SPINE_EXTERN void RegisterDxLibFunctions(const DxLibRegerenda* pDxLibRegerenda);
}

#if !defined(SPINE_RUNTIME_DLL_BUILD)
const DxLibRegerenda* GetDxLibFunctonsToBeRegistered();
#endif

#endif

