#ifndef Spine_TextureLoader_h
#define Spine_TextureLoader_h

#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
	class AtlasPage;

	class SP_API TextureLoader : public SpineObject {
	public:
		TextureLoader();

		virtual ~TextureLoader();

		virtual void load(AtlasPage &page, const String &path) = 0;

		virtual void unload(void *texture) = 0;
	};
}

#endif
