#ifndef Spine_Interpolation_h
#define Spine_Interpolation_h

#include <spine/SpineObject.h>

namespace spine {

	class SP_API Interpolation : public SpineObject {
	public:
		virtual ~Interpolation();

		virtual float apply(float a);

		float apply(float start, float end, float a);

		static Interpolation &linear();

		static Interpolation &smooth();

		static Interpolation &slowFast();

		static Interpolation &fastSlow();

		static Interpolation &circle();
	};
}

#endif
