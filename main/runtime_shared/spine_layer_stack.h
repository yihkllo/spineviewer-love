#ifndef SPINELOVE_RUNTIME_SHARED_SPINE_LAYER_STACK_H_
#define SPINELOVE_RUNTIME_SHARED_SPINE_LAYER_STACK_H_

#include <cstddef>

class SlLayerStack
{
public:
	virtual ~SlLayerStack() = default;

	virtual size_t LoadedSkeletonCount() const noexcept = 0;
	virtual size_t ActiveSkeletonIndex() const noexcept = 0;
	virtual bool ChooseSkeleton(size_t index) noexcept = 0;
	virtual bool SkeletonLayerVisible(size_t index) const noexcept = 0;
	virtual bool SetSkeletonLayerVisible(size_t index, bool visible) noexcept = 0;
	virtual bool PromoteSkeleton(size_t index) noexcept = 0;
	virtual bool DemoteSkeleton(size_t index) noexcept = 0;
};

#endif
