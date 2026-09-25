
#include <spine/TransformConstraint.h>
#include <spine/Skeleton.h>
#include <spine/extension.h>

spTransformConstraint* spTransformConstraint_create (spTransformConstraintData* data, const spSkeleton* skeleton) {
	spTransformConstraint* self = NEW(spTransformConstraint);
	CONST_CAST(spTransformConstraintData*, self->data) = data;
	self->translateMix = data->translateMix;
	self->x = data->x;
	self->y = data->y;
	self->bone = spSkeleton_findBone(skeleton, self->data->bone->name);
	self->target = spSkeleton_findBone(skeleton, self->data->target->name);
	return self;
}

void spTransformConstraint_dispose (spTransformConstraint* self) {
	FREE(self);
}

void spTransformConstraint_apply (spTransformConstraint* self) {
	if (self->translateMix > 0) {
		float tx, ty;
		spBone_localToWorld(self->target, self->x, self->y, &tx, &ty);
		CONST_CAST(float, self->bone->worldX) = self->bone->worldX + (tx - self->bone->worldX) * self->translateMix;
		CONST_CAST(float, self->bone->worldY) = self->bone->worldY + (ty - self->bone->worldY) * self->translateMix;
	}
}
