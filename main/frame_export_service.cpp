#include "frame_export_service.h"

#include <cstring>
#include <utility>

#include <DxLib.h>

namespace frame_export
{
	namespace
	{
		enum class EImageFormat
		{
			Jpeg,
			Png,
		};

		struct CaptureArea
		{
			int width = 0;
			int height = 0;
		};

		class ScreenSnapshotBuffer
		{
		public:
			~ScreenSnapshotBuffer()
			{
				Reset();
			}

			bool CaptureCurrentScreen(bool toRgba)
			{
				Reset();
				DxLib::GetScreenState(&m_area.width, &m_area.height, nullptr);
				if (m_area.width <= 0 || m_area.height <= 0)
					return false;

				m_softImageHandle = toRgba
					? DxLib::MakeABGR8ColorSoftImage(m_area.width, m_area.height)
					: DxLib::MakeARGB8ColorSoftImage(m_area.width, m_area.height);
				if (m_softImageHandle == -1)
					return false;

				return DxLib::GetDrawScreenSoftImage(0, 0, m_area.width, m_area.height, m_softImageHandle) != -1;
			}

			bool CopyTo(ScreenPixels& output) const
			{
				if (m_softImageHandle == -1)
					return false;

				auto* bytes = static_cast<unsigned char*>(DxLib::GetImageAddressSoftImage(m_softImageHandle));
				const int stride = DxLib::GetPitchSoftImage(m_softImageHandle);
				if (bytes == nullptr || stride == -1)
					return false;

				output.width = m_area.width;
				output.height = m_area.height;
				output.stride = stride;
				output.rgbaBytes.resize(static_cast<size_t>(stride * m_area.height));
				std::memcpy(output.rgbaBytes.data(), bytes, output.rgbaBytes.size());
				return true;
			}

		private:
			void Reset()
			{
				if (m_softImageHandle != -1)
					DxLib::DeleteSoftImage(m_softImageHandle);
				m_softImageHandle = -1;
				m_area = {};
			}

			int m_softImageHandle = -1;
			CaptureArea m_area{};
		};

		CaptureArea QueryScreenArea()
		{
			CaptureArea area{};
			DxLib::GetScreenState(&area.width, &area.height, nullptr);
			return area;
		}

		bool QueryTextureArea(int textureHandle, CaptureArea& area)
		{
			return DxLib::GetGraphSize(textureHandle, &area.width, &area.height) != -1;
		}

		bool WriteScreenArea(const CaptureArea& area, const wchar_t* filePath, EImageFormat format)
		{
			if (area.width <= 0 || area.height <= 0)
				return false;

			switch (format)
			{
			case EImageFormat::Jpeg:
				return DxLib::SaveDrawScreenToJPEG(0, 0, area.width, area.height, filePath) != -1;
			case EImageFormat::Png:
				return DxLib::SaveDrawScreenToPNG(0, 0, area.width, area.height, filePath) != -1;
			default:
				return false;
			}
		}

		bool WriteTextureArea(int textureHandle, const CaptureArea& area, const wchar_t* filePath, EImageFormat format)
		{
			if (area.width <= 0 || area.height <= 0)
				return false;

			switch (format)
			{
			case EImageFormat::Jpeg:
				return DxLib::SaveDrawValidGraphToJPEG(textureHandle, 0, 0, area.width, area.height, filePath) != -1;
			case EImageFormat::Png:
				return DxLib::SaveDrawValidGraphToPNG(textureHandle, 0, 0, area.width, area.height, filePath) != -1;
			default:
				return false;
			}
		}
	}

	bool CFrameExportService::ExportScreenJpeg(const wchar_t* filePath) const
	{
		return WriteScreenArea(QueryScreenArea(), filePath, EImageFormat::Jpeg);
	}

	bool CFrameExportService::ExportScreenPng(const wchar_t* filePath) const
	{
		return WriteScreenArea(QueryScreenArea(), filePath, EImageFormat::Png);
	}

	bool CFrameExportService::ExportTextureJpeg(int textureHandle, const wchar_t* filePath) const
	{
		CaptureArea area{};
		return QueryTextureArea(textureHandle, area) && WriteTextureArea(textureHandle, area, filePath, EImageFormat::Jpeg);
	}

	bool CFrameExportService::ExportTexturePng(int textureHandle, const wchar_t* filePath) const
	{
		CaptureArea area{};
		return QueryTextureArea(textureHandle, area) && WriteTextureArea(textureHandle, area, filePath, EImageFormat::Png);
	}

	bool CFrameExportService::ReadScreenPixels(ScreenPixels& output, bool toRgba) const
	{
		ScreenSnapshotBuffer snapshot;
		return snapshot.CaptureCurrentScreen(toRgba) && snapshot.CopyTo(output);
	}
}
