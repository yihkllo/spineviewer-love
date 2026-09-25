
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/AttachmentLoader.h>

#include <spine/Skin.h>
#include <spine/RegionAttachment.h>
#include <spine/MeshAttachment.h>
#include <spine/BoundingBoxAttachment.h>
#include <spine/PathAttachment.h>
#include <spine/PointAttachment.h>
#include <spine/ClippingAttachment.h>

using namespace spine;

RTTI_IMPL_NOPARENT(AttachmentLoader)

AttachmentLoader::AttachmentLoader() {
}

AttachmentLoader::~AttachmentLoader() {
}
