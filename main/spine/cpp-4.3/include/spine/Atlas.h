#ifndef Spine_Atlas_h
#define Spine_Atlas_h

#include <spine/Array.h>
#include <spine/Extension.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/HasRendererObject.h>
#include "TextureRegion.h"
#include "spine/MeshAttachment.h"
#include "spine/RegionAttachment.h"

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
	enum SpineTextureFilter {
		TextureFilter_Unknown,
		TextureFilter_Nearest,
		TextureFilter_Linear,
		TextureFilter_MipMap,
		TextureFilter_MipMapNearestNearest,
		TextureFilter_MipMapLinearNearest,
		TextureFilter_MipMapNearestLinear,
		TextureFilter_MipMapLinearLinear
	};
#else
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
#endif

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
#ifdef SPINE_UE4
		SpineTextureFilter minFilter;
		SpineTextureFilter magFilter;
#else
		TextureFilter minFilter;
		TextureFilter magFilter;
#endif
		TextureWrap uWrap;
		TextureWrap vWrap;
		int width, height;
		bool pma;
		int index;
		void *texture;

		explicit AtlasPage(const String &inName)
			: name(inName), format(Format_RGBA8888), minFilter(TextureFilter_Nearest), magFilter(TextureFilter_Nearest),
			  uWrap(TextureWrap_ClampToEdge), vWrap(TextureWrap_ClampToEdge), width(0), height(0), pma(false), index(0), texture(nullptr) {
		}
	};

	class SP_API AtlasRegion : public TextureRegion {
		friend class Atlas;
		friend class RegionAttachment;
		friend class MeshAttachment;

		RTTI_DECL

	public:
		AtlasRegion()
			: TextureRegion(), _page(nullptr), _name(""), _index(0), _x(0), _y(0), _offsetX(0), _offsetY(0), _packedWidth(0), _packedHeight(0),
			  _originalWidth(0), _originalHeight(0), _rotate(false), _degrees(0) {
		}
		~AtlasRegion() {
		}
		AtlasPage *getPage() const {
			return _page;
		}
		const String &getName() const {
			return _name;
		}
		int getIndex() const {
			return _index;
		}
		int getX() const {
			return _x;
		}
		int getY() const {
			return _y;
		}
		float getOffsetX() const {
			return _offsetX;
		}
		float getOffsetY() const {
			return _offsetY;
		}
		int getPackedWidth() const {
			return _packedWidth;
		}
		int getPackedHeight() const {
			return _packedHeight;
		}
		int getOriginalWidth() const {
			return _originalWidth;
		}
		int getOriginalHeight() const {
			return _originalHeight;
		}
		bool getRotate() const {
			return _rotate;
		}
		int getDegrees() const {
			return _degrees;
		}
		Array<int> &getSplits() {
			return _splits;
		}
		Array<int> &getPads() {
			return _pads;
		}
		Array<String> &getNames() {
			return _names;
		}
		Array<float> &getValues() {
			return _values;
		}
		void setPage(AtlasPage *value) {
			_page = value;
		}
		void setName(const String &value) {
			_name = value;
		}
		void setIndex(int value) {
			_index = value;
		}
		void setX(int value) {
			_x = value;
		}
		void setY(int value) {
			_y = value;
		}
		void setOffsetX(float value) {
			_offsetX = value;
		}
		void setOffsetY(float value) {
			_offsetY = value;
		}
		void setPackedWidth(int value) {
			_packedWidth = value;
		}
		void setPackedHeight(int value) {
			_packedHeight = value;
		}
		void setOriginalWidth(int value) {
			_originalWidth = value;
		}
		void setOriginalHeight(int value) {
			_originalHeight = value;
		}
		void setRotate(bool value) {
			_rotate = value;
		}
		void setDegrees(int value) {
			_degrees = value;
		}
		void setSplits(const Array<int> &value) {
			_splits = value;
		}
		void setPads(const Array<int> &value) {
			_pads = value;
		}
		void setNames(const Array<String> &value) {
			_names = value;
		}
		void setValues(const Array<float> &value) {
			_values = value;
		}

	private:
		AtlasPage *_page;
		String _name;
		int _index;
		int _x, _y;
		float _offsetX, _offsetY;
		int _packedWidth, _packedHeight;
		int _originalWidth, _originalHeight;
		bool _rotate;
		int _degrees;
		Array<int> _splits;
		Array<int> _pads;
		Array<String> _names;
		Array<float> _values;
	};

	class TextureLoader;

	class SP_API Atlas : public SpineObject {
	public:
		Atlas(const String &path, TextureLoader *textureLoader, bool createTexture = true);

		Atlas(const char *data, int length, const char *dir, TextureLoader *textureLoader, bool createTexture = true);

		~Atlas();

		void flipV();

		AtlasRegion *findRegion(const String &name);

		Array<AtlasPage *> &getPages();

		Array<AtlasRegion *> &getRegions();

	private:
		Array<AtlasPage *> _pages;
		Array<AtlasRegion *> _regions;
		TextureLoader *_textureLoader;

		void load(const char *begin, int length, const char *dir, bool createTexture);
	};
}

#endif
