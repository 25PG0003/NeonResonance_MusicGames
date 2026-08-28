#pragma once
#include <string>
#include <Windows.h>
#include <mmsystem.h>
#include "Gimmick.h"

#pragma comment(lib, "winmm.lib")

class Music
{
public:
    Music();
    ~Music();

    bool Load(const std::string& path);   // WAV ファイル読み込み
    void Play();
    void Stop();
    void Pause();
    void Resume();

    void Chart(const std::string& chartpath, const std::string& musicpath);

    float GetTime() const; // 再生経過時間（秒）

private:
    std::string filePath;
    std::string chartPath;
    bool isPlaying = false;
    bool isPaused = false;

    Gimmick chart;

    LARGE_INTEGER startTime;
    LARGE_INTEGER pauseTime;
    LARGE_INTEGER pausedDuration;
    LARGE_INTEGER freq;
};