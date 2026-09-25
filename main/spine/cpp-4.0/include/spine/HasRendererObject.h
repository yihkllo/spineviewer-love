
#ifndef Spine_HasRendererObject_h
#define Spine_HasRendererObject_h

#include <spine/dll.h>

namespace spine {

	typedef void (*DisposeRendererObject)(void *rendererObject);

	class SP_API HasRendererObject {
	public:
		explicit HasRendererObject() : _rendererObject(0), _dispose(0) {};

		virtual ~HasRendererObject() {
			if (_dispose && _rendererObject)
				_dispose(_rendererObject);
		}

		void *getRendererObject() { return _rendererObject; }

		void setRendererObject(void *rendererObject, DisposeRendererObject dispose = 0) {
			if (_dispose && _rendererObject && _rendererObject != rendererObject)
				_dispose(_rendererObject);

			_rendererObject = rendererObject;
			_dispose = dispose;
		}

	private:
		void *_rendererObject;
		DisposeRendererObject _dispose;
	};

}

#endif
