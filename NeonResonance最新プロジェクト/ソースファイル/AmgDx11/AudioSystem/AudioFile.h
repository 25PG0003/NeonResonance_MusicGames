#pragma once

#include <cstdint>

class AudioFile
{
protected:
	WAVEFORMATEX AudioFormat;
	int AudioBufferSize;
	BYTE* AudioBuffer;

public:
	AudioFile();
	~AudioFile();

	bool Load(const char* filename);

	WAVEFORMATEX* GetFormat() { return &AudioFormat; }
	BYTE* GetBuffer() { return AudioBuffer; }
	int GetBufferSize() { return AudioBufferSize; }
	uint32_t GetSampleRate()const { return AudioFormat.nSamplesPerSec; }

};

