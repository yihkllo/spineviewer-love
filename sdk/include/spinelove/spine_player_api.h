#ifndef SPINELOVE_RUNTIME_SHARED_SPINE_PLAYER_API_H_
#define SPINELOVE_RUNTIME_SHARED_SPINE_PLAYER_API_H_

#include "spinelove/spine_animation_deck.h"
#include "spinelove/spine_document.h"
#include "spinelove/spine_layer_stack.h"
#include "spinelove/spine_render_options.h"
#include "spinelove/spine_skin_deck.h"
#include "spinelove/spine_slot_probe.h"
#include "spinelove/spine_viewport_rig.h"

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
