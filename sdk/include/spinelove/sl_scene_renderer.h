#ifndef SPINELOVE_SL_SCENE_RENDERER_H_
#define SPINELOVE_SL_SCENE_RENDERER_H_

#include "spinelove/sl_gfx_draw.h"
#include <cstdint>
#include <unordered_map>

class SlSceneRenderer
{
public:
    virtual ~SlSceneRenderer() = default;
    virtual SlTextureId LoadTextureUtf8(const char* path, bool premultiplyAlpha) = 0;
    virtual void ReleaseTexture(SlTextureId texture) noexcept = 0;
    virtual void Submit(const SlDrawList& commands,
        const std::unordered_map<std::uint64_t, SlTextureId>& textures) = 0;
};

#endif
