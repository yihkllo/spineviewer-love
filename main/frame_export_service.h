#ifndef SPINELOVE_FRAME_EXPORT_SERVICE_H_
#define SPINELOVE_FRAME_EXPORT_SERVICE_H_

#include <vector>

namespace frame_export
{
	struct ScreenPixels
	{
		int width = 0;
		int height = 0;
		int stride = 0;
		std::vector<unsigned char> rgbaBytes;
	};

	class CFrameExportService final
	{
	public:
		bool ExportScreenJpeg(const wchar_t* filePath) const;
		bool ExportScreenPng(const wchar_t* filePath) const;

		bool ExportTextureJpeg(int textureHandle, const wchar_t* filePath) const;
		bool ExportTexturePng(int textureHandle, const wchar_t* filePath) const;

		bool ReadScreenPixels(ScreenPixels& output, bool toRgba = true) const;
	};
}

#endif

