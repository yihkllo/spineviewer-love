#pragma once
#include <string>

struct SlNativeMotionTrack
{
	int track = 0;
	std::string animation;
	bool loop = false;
	bool emptyAnimation = false;
	bool holdPrevious = false;
	float time = 0;
	float speed = 1;
	float mix = -1;
	float eventThreshold = 0.5f;
	float attachmentThreshold = 0.5f;
	float drawOrderThreshold = 0.5f;
};
