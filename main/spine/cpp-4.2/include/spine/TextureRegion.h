
#ifndef Spine_TextureRegion_h
#define Spine_TextureRegion_h

#include <spine/Vector.h>

namespace spine {
	class SP_API TextureRegion : public SpineObject {
	public:
		void *rendererObject;
		float u, v, u2, v2;
		int degrees;
		float offsetX, offsetY;
		int width, height;
		int originalWidth, originalHeight;

		TextureRegion(): rendererObject(NULL), u(0), v(0), u2(0), v2(0), degrees(0), offsetX(0), offsetY(0), width(0), height(0), originalWidth(0), originalHeight(0) {};
		~TextureRegion() {};
	};
}

#endif
