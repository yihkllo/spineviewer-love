
#ifndef Spine_Atlas_h
#define Spine_Atlas_h

#include <spine/Vector.h>
#include <spine/Extension.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/HasRendererObject.h>

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

enum TextureFilter {
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

class SP_API AtlasPage : public SpineObject, public HasRendererObject {
public:
	String name;
	String texturePath;
	Format format;
	TextureFilter minFilter;
	TextureFilter magFilter;
	TextureWrap uWrap;
	TextureWrap vWrap;
	int width, height;

	explicit AtlasPage(const String &inName) : name(inName), format(Format_RGBA8888), minFilter(TextureFilter_Nearest),
		magFilter(TextureFilter_Nearest), uWrap(TextureWrap_ClampToEdge),
		vWrap(TextureWrap_ClampToEdge), width(0), height(0) {
	}
};

class SP_API AtlasRegion : public SpineObject {
public:
	AtlasPage *page;
	String name;
	int x, y, width, height;
	float u, v, u2, v2;
	float offsetX, offsetY;
	int originalWidth, originalHeight;
	int index;
	bool rotate;
	int degrees;
	Vector<int> splits;
	Vector<int> pads;
};

class TextureLoader;

class SP_API Atlas : public SpineObject {
public:
	Atlas(const String &path, TextureLoader *textureLoader, bool createTexture = true);

	Atlas(const char *data, int length, const char *dir, TextureLoader *textureLoader, bool createTexture = true);

	~Atlas();

	void flipV();

	AtlasRegion *findRegion(const String &name);

	Vector<AtlasPage*> &getPages();

    Vector<AtlasRegion*> &getRegions();

private:
	Vector<AtlasPage *> _pages;
	Vector<AtlasRegion *> _regions;
	TextureLoader *_textureLoader;

	void load(const char *begin, int length, const char *dir, bool createTexture);

	class Str {
	public:
		const char *begin;
		const char *end;
	};

	static void trim(Str *str);

	static int readLine(const char **begin, const char *end, Str *str);

	static int beginPast(Str *str, char c);

	static int readValue(const char **begin, const char *end, Str *str);

	static int readTuple(const char **begin, const char *end, Str tuple[]);

	static char *mallocString(Str *str);

	static int indexOf(const char **array, int count, Str *str);

	static int equals(Str *str, const char *other);

	static int toInt(Str *str);
};
}

#endif
