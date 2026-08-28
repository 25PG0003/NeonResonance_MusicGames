#pragma once
#include <windows.h>
#include <xaudio2.h>
#include "AudioFile.h"

class Voice
{
public:
	bool Load(const char* filename, bool isLoop = false);
	void Release();

	void Play();
	void Stop();
	bool IsPlay();
	void SetBaseVolume(float vol);
	void ApplyVolume(float master);
	float GetPlayTime()const;
	void Seek(float sec);
	void SetVolume(float volume);
	void PlaySE();

	Voice();
	virtual ~Voice();

	
private:
	XAUDIO2_BUFFER AudioBuffer;
	XAUDIO2_BUFFER seekBuffer{};
	AudioFile wav;
	IXAudio2SourceVoice* SourceVoice;
	float baseVolume = 1.0f;
	float seekBaseTime = 0.0f;
	
	//éûä‘ä«óù
	float currentTime = 0.0f;
	bool playing = false;
	bool seeked = false;
	bool isReleased = false;
	uint64_t startSample = 0;
};

