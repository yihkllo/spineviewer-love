
#ifndef Spine_PathConstraint_h
#define Spine_PathConstraint_h

#include <spine/ConstraintData.h>

#include <spine/Vector.h>

namespace spine {
	class PathConstraintData;
	class Skeleton;
	class PathAttachment;
	class Bone;
	class Slot;

	class SP_API PathConstraint : public Updatable {
		friend class Skeleton;
		friend class PathConstraintMixTimeline;
		friend class PathConstraintPositionTimeline;
		friend class PathConstraintSpacingTimeline;

		RTTI_DECL

	public:
		PathConstraint(PathConstraintData& data, Skeleton& skeleton);

		void apply();

		virtual void update();

		virtual int getOrder();

		float getPosition();
		void setPosition(float inValue);

		float getSpacing();
		void setSpacing(float inValue);

		float getRotateMix();
		void setRotateMix(float inValue);

		float getTranslateMix();
		void setTranslateMix(float inValue);

		Vector<Bone*>& getBones();

		Slot* getTarget();
		void setTarget(Slot* inValue);

		PathConstraintData& getData();

		bool isActive();

		void setActive(bool inValue);

	private:
		static const float EPSILON;
		static const int NONE;
		static const int BEFORE;
		static const int AFTER;

		PathConstraintData& _data;
		Vector<Bone*> _bones;
		Slot* _target;
		float _position, _spacing, _rotateMix, _translateMix;

		Vector<float> _spaces;
		Vector<float> _positions;
		Vector<float> _world;
		Vector<float> _curves;
		Vector<float> _lengths;
		Vector<float> _segments;

		bool _active;

		Vector<float>& computeWorldPositions(PathAttachment& path, int spacesCount, bool tangents, bool percentPosition, bool percentSpacing);

		static void addBeforePosition(float p, Vector<float>& temp, int i, Vector<float>& output, int o);

		static void addAfterPosition(float p, Vector<float>& temp, int i, Vector<float>& output, int o);

		static void addCurvePosition(float p, float x1, float y1, float cx1, float cy1, float cx2, float cy2, float x2, float y2, Vector<float>& output, int o, bool tangents);
	};
}

#endif
