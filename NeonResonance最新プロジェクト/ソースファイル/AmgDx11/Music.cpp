#include "Music.h"
#include "Gimmick.h"

Music::Music()
{
    QueryPerformanceFrequency(&freq);
    pausedDuration.QuadPart = 0;
}

Music::~Music()
{
    Stop();
}

bool Music::Load(const std::string& path)
{
    filePath = path;
    return true; // WAV なので特にロード不要
}


void Music::Play()
{
    if (filePath.empty()) return;

    PlaySoundA(filePath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
    isPlaying = true;
    isPaused = false;

    QueryPerformanceCounter(&startTime);
    pausedDuration.QuadPart = 0;
}

void Music::Stop()
{
    PlaySound(NULL, 0, 0);
    isPlaying = false;
    isPaused = false;
    pausedDuration.QuadPart = 0;
}

void Music::Pause()
{
    if (!isPlaying || isPaused) return;

    PlaySound(NULL, 0, 0); // 一時停止
    isPaused = true;

    QueryPerformanceCounter(&pauseTime);
}

void Music::Resume()
{
    if (!isPlaying || !isPaused) return;

    PlaySoundA(filePath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
    isPaused = false;

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    pausedDuration.QuadPart += now.QuadPart - pauseTime.QuadPart;
}

float Music::GetTime() const
{
    if (!isPlaying) return 0.0f;

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    LONGLONG elapsed = now.QuadPart - startTime.QuadPart - pausedDuration.QuadPart;
    return static_cast<float>(elapsed) / static_cast<float>(freq.QuadPart);
}