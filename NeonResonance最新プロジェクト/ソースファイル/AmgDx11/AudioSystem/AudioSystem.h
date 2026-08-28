#pragma once

#include <windows.h>
//#include <xaudio2.h>
#include <xaudio2.h>
#include "AudioSource.h"

class AudioSystem
{
public:
	static IXAudio2* XAudio2;

	static bool Initialize();
	static void Release();

private:
	static IXAudio2MasteringVoice* MasteringVoice;
};

