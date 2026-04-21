#include "dll_bridge.h"

namespace
{
	class DxLibBridgeContext
	{
	public:
		void Bind(const DxLibRegerenda& api)
		{
			m_api = api;
		}

		const DxLibRegerenda& Api() const
		{
			return m_api;
		}

	private:
		DxLibRegerenda m_api{};
	};

	DxLibBridgeContext& BridgeContext()
	{
		static DxLibBridgeContext context;
		return context;
	}

#if !defined(SPINE_RUNTIME_DLL_BUILD)
	DxLibTextApi BuildTextApi()
	{
		DxLibTextApi api{};
#if defined(_WIN32) && defined(_UNICODE)
		api.GetUseCharCodeFormat = &DxLib::GetUseCharCodeFormat;
		api.Get_wchar_t_CharCodeFormat = &DxLib::Get_wchar_t_CharCodeFormat;
		api.ConvertStringCharCodeFormat = &DxLib::ConvertStringCharCodeFormat;
#endif
		return api;
	}

	DxLibGraphApi BuildGraphApi()
	{
		DxLibGraphApi api{};
		api.LoadGraph = &DxLib::LoadGraph;
		api.DeleteGraph = &DxLib::DeleteGraph;
		api.SetUsePremulAlphaConvertLoad = &DxLib::SetUsePremulAlphaConvertLoad;
		api.GetUsePremulAlphaConvertLoad = &DxLib::GetUsePremulAlphaConvertLoad;
		api.SetDrawCustomBlendMode = &DxLib::SetDrawCustomBlendMode;
		api.SetDrawBlendMode = &DxLib::SetDrawBlendMode;
		api.DrawPolygonIndexed2D = &DxLib::DrawPolygonIndexed2D;
		return api;
	}

	DxLibTransformApi BuildTransformApi()
	{
		DxLibTransformApi api{};
		api.GetDrawScreenSize = &DxLib::GetDrawScreenSize;
		api.MGetScale = &DxLib::MGetScale;
		api.MGetTranslate = &DxLib::MGetTranslate;
		api.MGetRotZ = &DxLib::MGetRotZ;
		api.MMult = &DxLib::MMult;
		api.SetTransformTo2D = &DxLib::SetTransformTo2D;
		api.ResetTransformTo2D = &DxLib::ResetTransformTo2D;
		return api;
	}

	DxLibDisplayApi BuildDisplayApi()
	{
		DxLibDisplayApi api{};
#if defined _WIN32
		api.GetDisplayMaxResolution = &DxLib::GetDisplayMaxResolution;
#elif defined __ANDROID__
		api.GetAndroidDisplayResolution = &DxLib::GetAndroidDisplayResolution;
#elif defined __APPLE__
		api.GetDisplayResolution_iOS = &DxLib::GetDisplayResolution_iOS;
#endif
		return api;
	}
#endif
}

#if defined(SPINE_RUNTIME_DLL_BUILD)
namespace DxLib
{
#if defined(_WIN32) && defined(_UNICODE)
	extern int GetUseCharCodeFormat(void)
	{
		return BridgeContext().Api().text.GetUseCharCodeFormat();
	}

	extern int Get_wchar_t_CharCodeFormat(void)
	{
		return BridgeContext().Api().text.Get_wchar_t_CharCodeFormat();
	}

	extern int ConvertStringCharCodeFormat(int SrcCharCodeFormat, const void* SrcString, int DestCharCodeFormat, void* DestStringBuffer)
	{
		return BridgeContext().Api().text.ConvertStringCharCodeFormat(SrcCharCodeFormat, SrcString, DestCharCodeFormat, DestStringBuffer);
	}
#endif

	extern int LoadGraph(const TCHAR* FileName, int NotUse3DFlag)
	{
		return BridgeContext().Api().graph.LoadGraph(FileName, NotUse3DFlag);
	}

	extern int DeleteGraph(int GrHandle)
	{
		return BridgeContext().Api().graph.DeleteGraph(GrHandle);
	}

	extern int SetUsePremulAlphaConvertLoad(int UseFlag)
	{
		return BridgeContext().Api().graph.SetUsePremulAlphaConvertLoad(UseFlag);
	}

	extern int GetUsePremulAlphaConvertLoad(void)
	{
		return BridgeContext().Api().graph.GetUsePremulAlphaConvertLoad();
	}

	extern int SetDrawCustomBlendMode(int BlendEnable, int SrcBlendRGB, int DestBlendRGB, int BlendOpRGB, int SrcBlendA, int DestBlendA, int BlendOpA, int BlendParam)
	{
		return BridgeContext().Api().graph.SetDrawCustomBlendMode(BlendEnable, SrcBlendRGB, DestBlendRGB, BlendOpRGB, SrcBlendA, DestBlendA, BlendOpA, BlendParam);
	}

	extern int SetDrawBlendMode(int BlendMode, int BlendParam)
	{
		return BridgeContext().Api().graph.SetDrawBlendMode(BlendMode, BlendParam);
	}

	extern int DrawPolygonIndexed2D(const VERTEX2D* VertexArray, int VertexNum, const unsigned short* IndexArray, int PolygonNum, int GrHandle, int TransFlag)
	{
		return BridgeContext().Api().graph.DrawPolygonIndexed2D(VertexArray, VertexNum, IndexArray, PolygonNum, GrHandle, TransFlag);
	}

	extern int GetDrawScreenSize(int* XBuf, int* YBuf)
	{
		return BridgeContext().Api().transform.GetDrawScreenSize(XBuf, YBuf);
	}

	extern MATRIX MGetScale(VECTOR Scale)
	{
		return BridgeContext().Api().transform.MGetScale(Scale);
	}

	extern MATRIX MGetTranslate(VECTOR Trans)
	{
		return BridgeContext().Api().transform.MGetTranslate(Trans);
	}

	extern MATRIX MGetRotZ(float angle)
	{
		return BridgeContext().Api().transform.MGetRotZ(angle);
	}

	extern MATRIX MMult(MATRIX In1, MATRIX In2)
	{
		return BridgeContext().Api().transform.MMult(In1, In2);
	}

	extern int SetTransformTo2D(const MATRIX* Matrix)
	{
		return BridgeContext().Api().transform.SetTransformTo2D(Matrix);
	}

	extern int ResetTransformTo2D(void)
	{
		return BridgeContext().Api().transform.ResetTransformTo2D();
	}

#if defined _WIN32
	extern int GetDisplayMaxResolution(int* SizeX, int* SizeY, int DisplayIndex)
	{
		return BridgeContext().Api().display.GetDisplayMaxResolution(SizeX, SizeY, DisplayIndex);
	}
#elif defined __ANDROID__
	extern int GetAndroidDisplayResolution(int* SizeX, int* SizeY)
	{
		return BridgeContext().Api().display.GetAndroidDisplayResolution(SizeX, SizeY);
	}
#elif defined __APPLE__
	extern int GetDisplayResolution_iOS(int* SizeX, int* SizeY)
	{
		return BridgeContext().Api().display.GetDisplayResolution_iOS(SizeX, SizeY);
	}
#endif
}

void RegisterDxLibFunctions(const DxLibRegerenda* pDxLibRegerenda)
{
	if (pDxLibRegerenda != nullptr)
		BridgeContext().Bind(*pDxLibRegerenda);
}

#else
const DxLibRegerenda* GetDxLibFunctonsToBeRegistered()
{
	static DxLibRegerenda s_dxLibRegerenda;
	s_dxLibRegerenda.text = BuildTextApi();
	s_dxLibRegerenda.graph = BuildGraphApi();
	s_dxLibRegerenda.transform = BuildTransformApi();
	s_dxLibRegerenda.display = BuildDisplayApi();
	return &s_dxLibRegerenda;
}
#endif
