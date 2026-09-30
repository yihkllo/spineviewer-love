#ifndef Spine_PosedActive_h
#define Spine_PosedActive_h

#include <spine/dll.h>

namespace spine {

	class SP_API PosedActive {
	protected:
		bool _active;

	public:
		PosedActive() : _active(true) {
		}
		virtual ~PosedActive() {
		}

		bool isActive() const {
			return _active;
		}
		void setActive(bool active) {
			_active = active;
		}
	};
}

#endif
