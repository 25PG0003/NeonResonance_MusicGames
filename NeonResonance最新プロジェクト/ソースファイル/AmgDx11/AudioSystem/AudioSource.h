#pragma once
#include <string>
#include "AudioVoice.h"
#include <map>

class AudioSource {
public:
	bool Load(std::string name, const char* filename, bool isLoop = false);
	void ReleaseVoice(std::string name);
	void ReleaseAllVoice();

	void Play(std::string name);
	void Stop(std::string name);
	bool IsPlay(std::string name);
	void Volume(const std::string& name,float vol);
	void SetMasterVolume(float vol);
	float GetPlayTime(const std::string& name);
	void Seek(const std::string& name, float sec);
	void PlaySE(std::string name);


	AudioSource() {
		voices.clear();
	}
	virtual ~AudioSource();

private:
	std::map<std::string, Voice> voices;
	Voice* GetVoice(std::string name);
	float masterVolume = 1.0f;
};

