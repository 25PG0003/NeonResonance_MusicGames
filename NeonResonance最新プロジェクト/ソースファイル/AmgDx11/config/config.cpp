#include "config.h"
#include <fstream>
#include <Windows.h>

float Config::bgmVolume = 1.0f;
float Config::seVolume = 1.0f;

bool Config::fullscreen = false;

float Config::notespeed = 2.0f;
int Config::currentPattern = 0;

int Config::LaneAngle = 6;

int Config::laneKey_pt1[5] =
{
    'S',
    'D',
    'J',
    'K',
    'L'
};

int Config::laneKey_pt2[5] =
{
    'S',
    'D',
    'F',
    'J',
    'K'
};

float Config::Offset = 0.0f;

void Config::Load()
{
    std::ifstream file("Config/config.ini");

    if (!file.is_open()) return;

    file >> bgmVolume;
    file >> seVolume;
    file >> fullscreen;
    file >> notespeed;
    file >> LaneAngle;
    file >> currentPattern;

    // PT1
    for (int i = 0; i < 5; i++)
    {
        file >> laneKey_pt1[i];
    }

    // PT2
    for (int i = 0; i < 5; i++)
    {
        file >> laneKey_pt2[i];
    }

    file >> Offset;

    file.close();
}

void Config::Save()
{
    std::ofstream file("Config/config.ini");

    if (!file.is_open())
    {
        OutputDebugStringA("Config load FAILED\n");
        return;
    }

    OutputDebugStringA("Config load OK\n");

    file << bgmVolume << std::endl;
    file << seVolume << std::endl;
    file << fullscreen << std::endl;
    file << notespeed << std::endl;
    file << LaneAngle << std::endl;
    file << currentPattern << std::endl;


    // PT1
    for (int i = 0; i < 5; i++)
    {
        file << laneKey_pt1[i] << std::endl;
    }

    // PT2
    for (int i = 0; i < 5; i++)
    {
        file << laneKey_pt2[i] << std::endl;
    }

    file << Offset << std::endl;

    file.close();
}

int* Config::GetCurrentLaneKeys()
{
    if (currentPattern == 0)
    {
        return laneKey_pt1;
    }
    return laneKey_pt2;
}