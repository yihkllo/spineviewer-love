#ifndef SPINELOVE_RUNTIME_SHARED_SPINE_SKIN_DECK_H_
#define SPINELOVE_RUNTIME_SHARED_SPINE_SKIN_DECK_H_

#include <string>
#include <vector>

#include "spine_animation_deck.h"

class SlLookDeck
{
public:
	virtual ~SlLookDeck() = default;

	virtual void StepToNextLook() = 0;
	virtual void ApplyLookByIndex(size_t index) = 0;
	virtual void ApplyLookByName(const char* lookId) = 0;
	virtual void ComposeLooks(const SlNameList& looks) = 0;
	virtual std::string ActiveLookName() = 0;
	virtual const SlNameList& LookNames() const noexcept = 0;
};

#endif
