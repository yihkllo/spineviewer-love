#ifndef SPINELOVE_RUNTIME_SHARED_SPINE_PLAYER_API_H_
#define SPINELOVE_RUNTIME_SHARED_SPINE_PLAYER_API_H_

#include "spine_animation_deck.h"
#include "spine_document.h"
#include "spine_layer_stack.h"
#include "spine_render_options.h"
#include "spine_skin_deck.h"
#include "spine_slot_probe.h"
#include "spine_viewport_rig.h"

class SlPlaybackRuntime :
	public SlAssetIntake,
	public SlLayerStack,
	public SlMotionDeck,
	public SlLookDeck,
	public SlRenderOptions,
	public SlViewportRig,
	public SlSlotProbe
{
public:
	virtual ~SlPlaybackRuntime() = default;
};

#endif
