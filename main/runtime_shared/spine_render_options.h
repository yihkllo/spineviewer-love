#ifndef SPINELOVE_RUNTIME_SHARED_SPINE_RENDER_OPTIONS_H_
#define SPINELOVE_RUNTIME_SHARED_SPINE_RENDER_OPTIONS_H_

#include <cstddef>
#include <string>
#include <vector>

class SlRenderOptions
{
public:
	virtual ~SlRenderOptions() = default;

	virtual void TogglePremultiplyMode() = 0;
	virtual bool UsesPremultipliedAlpha(size_t drawableIndex = 0) = 0;
	virtual bool SetPremultipliedAlpha(bool premultiplied, size_t drawableIndex = 0) = 0;
	virtual void SetHiddenSlots(const std::vector<std::string>& slotNames) = 0;
	virtual void SetSlotVisibilityRule(bool (*predicate)(const char*, size_t)) = 0;
};

#endif
