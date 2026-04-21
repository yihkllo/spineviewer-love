#ifndef BITMAP_IO_H_
#define BITMAP_IO_H_

#include <cstdint>
#include <memory>
#include <vector>

namespace win_image
{
	struct SImageFrame
	{
		uint32_t uiWidth  = 0;
		uint32_t uiHeight = 0;
		uint32_t uiStride = 0;
		std::vector<unsigned char> pixels;
	};

	enum class ERotation
	{
		None, Deg90, Deg180, Deg270
	};

	bool LoadImageToMemory(const wchar_t* filePath, SImageFrame* pImageFrame, float fScale = 1.f, ERotation rotation = ERotation::None);

	bool LoadImageToWicBitmap(const wchar_t* filePath, void** pWicBitmap, float fScale = 1.f, ERotation rotation = ERotation::None);

	bool SkimImageSize(const wchar_t* filePath, unsigned int* width, unsigned int* height);

	bool SaveImageAsPng(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha);
	bool SaveImageAsJpg(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha);

	class CWicGifEncoder
	{
	public:
		CWicGifEncoder();
		~CWicGifEncoder();

		CWicGifEncoder(const CWicGifEncoder&) = delete;
		CWicGifEncoder& operator=(const CWicGifEncoder&) = delete;

		bool Initialise(const wchar_t* filePath);
		bool HasBeenInitialised() const;

		bool CommitFrame(unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha, float delayInSeconds);

		bool Finalise();
	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}
#endif
