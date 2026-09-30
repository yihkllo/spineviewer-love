#ifndef Spine_TextureRegion_h
#define Spine_TextureRegion_h

#include <spine/Array.h>
#include <spine/RTTI.h>

namespace spine {
	class SP_API TextureRegion : public SpineObject {
		friend class MeshAttachment;
		friend class RegionAttachment;
		friend class Atlas;
		friend class AtlasRegion;
		friend class SkeletonRenderer;

		RTTI_DECL_NOPARENT

	public:
		TextureRegion() : _rendererObject(nullptr), _u(0), _v(0), _u2(0), _v2(0), _regionWidth(0), _regionHeight(0) {};
		~TextureRegion() {};

		float getU() const {
			return _u;
		};
		void setU(float value) {
			_u = value;
		}
		float getV() const {
			return _v;
		}
		void setV(float value) {
			_v = value;
		}
		float getU2() const {
			return _u2;
		}
		void setU2(float value) {
			_u2 = value;
		}
		float getV2() const {
			return _v2;
		}
		void setV2(float value) {
			_v2 = value;
		}
		int getRegionWidth() const {
			return _regionWidth;
		};
		void setRegionWidth(int value) {
			_regionWidth = value;
		}
		int getRegionHeight() const {
			return _regionHeight;
		}
		void setRegionHeight(int value) {
			_regionHeight = value;
		}
		void *getRendererObject() const {
			return _rendererObject;
		}
		void setRendererObject(void *value) {
			_rendererObject = value;
		}

	private:
		void *_rendererObject;
		float _u, _v, _u2, _v2;
		int _regionWidth, _regionHeight;
	};
}

#endif
