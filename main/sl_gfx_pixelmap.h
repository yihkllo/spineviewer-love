#ifndef SL_GFX_PIXELMAP_H_
#define SL_GFX_PIXELMAP_H_


class CDxLibMap
{
public:
	CDxLibMap(int iTextureHandle);
	~CDxLibMap();

	bool IsAccessible() const;

	int width = 0;
	int height = 0;
	int stride = 0;
	unsigned char* pPixels = nullptr;


	void* pColorData = nullptr;
private:
	int m_imageHandle = -1;
	bool m_isLocked = false;

	bool ReadPixels();
	void Unlock() const;
};

#endif
