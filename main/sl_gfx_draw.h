#ifndef SPINELOVE_SL_GFX_DRAW_H_
#define SPINELOVE_SL_GFX_DRAW_H_

#include <cstdint>
#include <string>
#include <vector>

#include "sl_gfx_types.h"

enum class SlBlendMode
{
	Normal,
	Additive,
	Multiply,
	Screen,
};

struct SlDrawCommand
{
	std::uint64_t textureId = 0;
	std::string slotName;
	SlBlendMode blendMode = SlBlendMode::Normal;
	bool premultipliedAlpha = false;
	std::vector<SlVertex2D> vertices;
	std::vector<unsigned short> indices;
};

struct SlDrawList
{
	int width = 0;
	int height = 0;
	std::vector<SlDrawCommand> commands;
};

#endif
