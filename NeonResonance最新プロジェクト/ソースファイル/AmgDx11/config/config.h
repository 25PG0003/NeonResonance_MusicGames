#pragma once

#include <filesystem>


class Config
{
public:

	static void Load();
	static void Save();
	static int* GetCurrentLaneKeys();

public:
	static float bgmVolume;
	static float seVolume;

	static bool fullscreen;
	static float notespeed;
	static float Offset;

	//key config
	static int laneKey_pt1[5];
	static int laneKey_pt2[5];
	static int currentPattern;

	static int LaneAngle;

};