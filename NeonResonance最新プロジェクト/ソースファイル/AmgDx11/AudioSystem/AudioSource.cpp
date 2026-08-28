#include "AudioSystem.h"
#include <string>

AudioSource::~AudioSource()
{
}

bool AudioSource::Load(std::string name, const char * filename, bool isLoop)
{
	Voice* voice = GetVoice(name);
	if (voice != NULL)
	{
		voice->Release();
	}

	bool result = voices[name].Load(filename, isLoop);

	if (!result)
	{
		voices.erase(name);
	}

	return result;
}

void AudioSource::ReleaseVoice(std::string name)
{
	Voice* voice = GetVoice(name);
	if (voice != NULL) {
		voice->Release();
		voices.erase(name);
	}
}

void AudioSource::ReleaseAllVoice()
{
	for (auto v = voices.begin(); v != voices.end(); v++) {
		v->second.Release();
	}
	voices.clear();
}

void AudioSource::Play(std::string name)
{
	Voice* voice = GetVoice(name);
	if (voice != NULL) {
		voice->Play();
	}
}

void AudioSource::PlaySE(std::string name)
{
	Voice* voice = GetVoice(name);
	if (voice != NULL)
	{
		voice->PlaySE();
	}
}

void AudioSource::Stop(std::string name)
{
	Voice* voice = GetVoice(name);
	if (voice != NULL) {
		voice->Stop();
	}
}

bool AudioSource::IsPlay(std::string name)
{
	Voice* voice = GetVoice(name);
	if (voice != NULL) {
		return voice->IsPlay();
	}
	return false;
}

Voice* AudioSource::GetVoice(std::string name)
{
	if (voices.count(name) == 0) {
		return NULL;
	}

	return &voices[name];
}

void AudioSource::Volume(const std::string& name, float vol)
{
	Voice* v = GetVoice(name);
	if (!v)return;
	v->SetBaseVolume(vol);
	v->ApplyVolume(masterVolume);
}

void AudioSource::SetMasterVolume(float vol)
{
	masterVolume = vol;

	for (auto& [name, voice] : voices)
	{
		voice.ApplyVolume(masterVolume);
	}
}

float AudioSource::GetPlayTime(const std::string& name)
{
	// マップから Voice を検索
	auto it = voices.find(name);
	if (it == voices.end()) {
		// 見つからなければ 0.0f を返す
		return 0.0f;
	}

	Voice* voice = &it->second; // アドレスを取得
	if (!voice->IsPlay())      // 再生中でなければ 0.0f
		return 0.0f;

	// 再生時間を返す
	return voice->GetPlayTime();
}

void AudioSource::Seek(const std::string& name, float sec)
{
	auto it = voices.find(name);

	if (it == voices.end())return;

	it->second.Seek(sec);
}