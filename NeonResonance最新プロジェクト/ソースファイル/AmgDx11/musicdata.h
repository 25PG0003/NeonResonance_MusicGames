#pragma once
#include <string>

struct MusicData
{
	std::string name;
	std::string jacketPath;
	std::string musicPath;
	float previewTime = 0.0f;

	std::string chartPath[3];

	float bpm = 0.0f;
	float offset = 0.0f;
};