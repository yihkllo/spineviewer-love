#ifndef MEDIA_MUX_H_
#define MEDIA_MUX_H_

#include <memory>

class CMfVideoEncoder
{
public:
	CMfVideoEncoder();
	~CMfVideoEncoder();

	CMfVideoEncoder(const CMfVideoEncoder&) = delete;
	CMfVideoEncoder& operator=(const CMfVideoEncoder&) = delete;


	bool initialise(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int frameRate);
	bool hasBeenInitialised() const;


	bool commitCpuFrame(unsigned char* pPixels, unsigned long pixelSize, bool isRgba = true);
#ifdef MF_GPU_TEXTURE

	bool commitGpuFrame(void* pD3D11Texture2D);
#endif
	void finalise();
private:
	static constexpr unsigned int DefaultFrameRate = 60;

	class Impl;
	std::unique_ptr<Impl> m_pImpl;
};
#endif
