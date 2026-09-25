
#ifndef Spine_Constraint_h
#define Spine_Constraint_h

#include <spine/Updatable.h>
#include <spine/SpineString.h>

namespace spine {
	class SP_API ConstraintData : public SpineObject {

	public:
		ConstraintData(const String &name);

		virtual ~ConstraintData();

		const String &getName();

		size_t getOrder();

		void setOrder(size_t inValue);

		bool isSkinRequired();

		void setSkinRequired(bool inValue);

	private:
		const String _name;
		size_t _order;
		bool _skinRequired;
	};
}

#endif
