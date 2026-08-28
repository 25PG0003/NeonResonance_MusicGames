#include "AudioSystem.h"

#include <mmsystem.h>
#pragma comment( lib, "xaudio2.lib" )
#pragma comment( lib, "winmm.lib" )

IXAudio2* AudioSystem::XAudio2 = NULL;
IXAudio2MasteringVoice* AudioSystem::MasteringVoice;

bool AudioSystem::Initialize()
{
	UINT32 flags = 0;	//XAUDIO2_DEBUG_ENGINE;
	if ( XAudio2Create(&XAudio2, flags) < 0 )
	{
		return false;
	}

	//	マスタリングボイス作成
	MasteringVoice = NULL;
	if ( XAudio2->CreateMasteringVoice(&MasteringVoice) < 0 )
	{
		XAudio2->Release();
		XAudio2 = NULL;
		return false;
	}

	return true;
}

void AudioSystem::Release()
{
	if (MasteringVoice) {
		MasteringVoice->DestroyVoice();
	}
	if (XAudio2) {
		XAudio2->Release();
		XAudio2 = NULL;

	}
}

