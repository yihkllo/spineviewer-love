
#ifndef Spine_Atlas_h
#define Spine_Atlas_h

#include <spine/Vector.h>
#include <spine/Extension.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/HasRendererObject.h>
#include "TextureRegion.h"

namespace spine {
	enum Format {
		Format_Alpha,
		Format_Intensity,
		Format_LuminanceAlpha,
		Format_RGB565,
		Format_RGBA4444,
		Format_RGB888,
		Format_RGBA8888
	};

#ifdef SPINE_UE4
	#define TEXTURE_FILTER_ENUM SpineTextureFilter
#else
	#define TEXTURE_FILTER_ENUM TextureFilter
#endif

	enum TEXTURE_FILTER_ENUM {
		TextureFilter_Unknown,
		TextureFilter_Nearest,
		TextureFilter_Linear,
		TextureFilter_MipMap,
		TextureFilter_MipMapNearestNearest,
		TextureFilter_MipMapLinearNearest,
		TextureFilter_MipMapNearestLinear,
		TextureFilter_MipMapLinearLinear
	};

	enum TextureWrap {
		TextureWrap_MirroredRepeat,
		TextureWrap_ClampToEdge,
		TextureWrap_Repeat
	};

	class SP_API AtlasPage : public SpineObject {
	public:
		String name;
		String texturePath;
		Format format;
		TEXTURE_FILTER_ENUM minFilter;
		TEXTURE_FILTER_ENUM magFilter;
		TextureWrap uWrap;
		TextureWrap vWrap;
		int width, height;
		bool pma;
        int index;
        void *texture;

		explicit AtlasPage(const String &inName) : name(inName), format(Format_RGBA8888),
												   minFilter(TextureFilter_Nearest),
												   magFilter(TextureFilter_Nearest), uWrap(TextureWrap_ClampToEdge),
												   vWrap(TextureWrap_ClampToEdge), width(0), height(0), pma(false), index(0), texture(NULL) {
		}
	};

	class SP_API AtlasRegion : public TextureRegion {
	public:
		AtlasPage *page;
		String name;
		int index;
		int x, y;
		Vector<int> splits;
		Vector<int> pads;
		Vector <String> names;
		Vector<float> values;
	};

	class TextureLoader;

	class SP_API Atlas : public SpineObject {
	public:
		Atlas(const String &path, TextureLoader *textureLoader, bool createTexture = true);

		Atlas(const char *data, int length, const char *dir, TextureLoader *textureLoader, bool createTexture = true);

		~Atlas();

		void flipV();

		AtlasRegion *findRegion(const String &name);

		Vector<AtlasPage *> &getPages();

		Vector<AtlasRegion *> &getRegions();

	private:
		Vector<AtlasPage *> _pages;
		Vector<AtlasRegion *> _regions;
		TextureLoader *_textureLoader;

		void load(const char *begin, int length, const char *dir, bool createTexture);
	};
}

#endif
