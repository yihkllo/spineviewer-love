#ifndef SPINELOVE_IMAGE_EXPORTER_H_
#define SPINELOVE_IMAGE_EXPORTER_H_

#include <string>

#include "sl_gfx_types.h"

enum class SlExportImageFormat
{
	Png,
	Jpeg,
};

enum class SlExportMovieFormat
{
	Mp4,
	Webm,
	Gif,
};

bool SaveExportImage(
	const wchar_t* path,
	SlExportImageFormat format,
	const unsigned char* premultipliedRgba,
	int width,
	int height,
	int stride,
	bool keepAlpha,
	const SlColor& matteColor,
	std::wstring* errorMessage = nullptr);

#endif
