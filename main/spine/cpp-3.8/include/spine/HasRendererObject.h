
#ifndef Spine_HasRendererObject_h
#define Spine_HasRendererObject_h

namespace spine {

typedef void (*DisposeRendererObject) (void* rendererObject);

class SP_API HasRendererObject {
public:
	explicit HasRendererObject() : _rendererObject(NULL), _dispose(NULL) {};

	virtual ~HasRendererObject() {
		if (_dispose && _rendererObject)
			_dispose(_rendererObject);
	}

	void* getRendererObject() { return _rendererObject; }
	void setRendererObject(void* rendererObject, DisposeRendererObject dispose = NULL) {
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
