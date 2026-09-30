#include <spine/DrawOrder.h>

#include <spine/Slot.h>

using namespace spine;

DrawOrder::DrawOrder(Array<Slot *> &setupPose) : _setupPose(setupPose), _pose(), _constrainedPose(), _appliedPose(&_pose) {
}

void DrawOrder::setupPose() {
	_pose.clear();
	_pose.setSize(_setupPose.size(), NULL);
	for (size_t i = 0, n = _setupPose.size(); i < n; ++i) {
		_pose[i] = _setupPose[i];
	}
}

Array<Slot *> &DrawOrder::getPose() {
	return _pose;
}

Array<Slot *> &DrawOrder::getAppliedPose() {
	return *_appliedPose;
}

void DrawOrder::unconstrained() {
	_appliedPose = &_pose;
}

void DrawOrder::constrained() {
	_appliedPose = &_constrainedPose;
}

void DrawOrder::reset() {
	_constrainedPose.clear();
	_constrainedPose.setSize(_pose.size(), NULL);
	for (size_t i = 0, n = _pose.size(); i < n; ++i) {
		_constrainedPose[i] = _pose[i];
	}
}
