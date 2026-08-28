#include "AudioSystem.h"

bool Voice::Load(const char* filename, bool isLoop)
{
	if (SourceVoice)
	{
		SourceVoice->Stop();
		SourceVoice->FlushSourceBuffers();
		SourceVoice->DestroyVoice();
		SourceVoice = nullptr;
	}


	isReleased = false;
	if (wav.Load(filename) == false) {
		SourceVoice = NULL;
		return false;
	}

	WAVEFORMATEX* wfx = wav.GetFormat();
	BYTE* pbWaveData = wav.GetBuffer();

	AudioSystem::XAudio2->CreateSourceVoice(&SourceVoice, wfx);

	ZeroMemory(&AudioBuffer, sizeof(XAUDIO2_BUFFER));
	AudioBuffer.pAudioData = pbWaveData;
	AudioBuffer.AudioBytes = wav.GetBufferSize();
	AudioBuffer.Flags = XAUDIO2_END_OF_STREAM;  // tell the source voice not to expect any data after this buffer
	if (isLoop) {
		AudioBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	SourceVoice->SubmitSourceBuffer(&AudioBuffer);

	return true;
}

void Voice::Release()
{
	if (!SourceVoice || isReleased) return;
	SourceVoice->Stop();

	SourceVoice->FlushSourceBuffers();

	SourceVoice->Discontinuity();

	SourceVoice->DestroyVoice();

	SourceVoice = nullptr;
	isReleased = true;
}

void Voice::Play()
{
	if (!SourceVoice || isReleased) return;

	SourceVoice->Start();
	playing = true;
}

void Voice::PlaySE()
{
	if (!SourceVoice || isReleased) return;

	SourceVoice->Stop();
	SourceVoice->FlushSourceBuffers();

	XAUDIO2_BUFFER playBuffer = AudioBuffer;
	playBuffer.PlayBegin = 0;
	playBuffer.PlayLength = 0;

	SourceVoice->SubmitSourceBuffer(&playBuffer);
	SourceVoice->Start();

	seekBaseTime = 0.0f;
	playing = true;
	seeked = false;
}


void Voice::Stop()
{
	if (!SourceVoice || isReleased) return;

	SourceVoice->Stop();

	playing = false;
}

bool Voice::IsPlay()
{
	if (!SourceVoice || isReleased) return false;

	XAUDIO2_VOICE_STATE state{};
	SourceVoice->GetState(&state);

	return state.BuffersQueued > 0;
}

void Voice::SetBaseVolume(float vol)
{
	baseVolume = vol;
}

void Voice::ApplyVolume(float master)
{
	if (SourceVoice)
	{
		SourceVoice->SetVolume(baseVolume * master);
	}
}

float Voice::GetPlayTime()const
{
	if (!SourceVoice || isReleased)return 0.0f;

	XAUDIO2_VOICE_STATE state{};
	SourceVoice->GetState(&state);

	uint64_t samples = state.SamplesPlayed - startSample;

	float time = static_cast<float>(samples) /
		static_cast<float>(wav.GetSampleRate());

	return seekBaseTime + time;
}


void Voice::Seek(float sec)
{
	if (!SourceVoice || isReleased) return;

	SourceVoice->Stop();
	SourceVoice->FlushSourceBuffers();

	const WAVEFORMATEX* wfx = wav.GetFormat();
	uint32_t sampleRate = wfx->nSamplesPerSec;
	uint32_t totalSamples = wav.GetBufferSize() / wfx->nBlockAlign;

	uint32_t begin = static_cast<uint32_t>(sec * sampleRate);

	if (begin >= totalSamples)
		begin = (totalSamples > 0) ? totalSamples - 1 : 0;

	seekBuffer = AudioBuffer;
	seekBuffer.PlayBegin = begin;

	SourceVoice->SubmitSourceBuffer(&seekBuffer);

	seekBaseTime = sec;

	XAUDIO2_VOICE_STATE state{};
	SourceVoice->GetState(&state);
	startSample = state.SamplesPlayed;

	playing = false;
	seeked = true;
}

void Voice::SetVolume(float volume)
{
	if (!SourceVoice) return;

	if (volume < 0.0f) volume = 0.0f;
	if (volume > 1.0f) volume = 1.0f;

	SourceVoice->SetVolume(volume);
}



Voice::Voice() { SourceVoice = nullptr; isReleased = false; }
Voice::~Voice()
{
}

