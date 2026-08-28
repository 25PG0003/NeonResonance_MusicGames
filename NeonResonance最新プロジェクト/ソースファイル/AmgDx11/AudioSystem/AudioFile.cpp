#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <mmsystem.h>

#include "AudioFile.h"
#include <stdio.h>


AudioFile::AudioFile()
{
	AudioBuffer = NULL;
}


AudioFile::~AudioFile()
{
	if (AudioBuffer != NULL) delete[] AudioBuffer;
}


bool AudioFile::Load(const char * filename)
{
	wchar_t file[128];
	mbstowcs(file, filename, 128);

	//	ファイルオープン
	HMMIO hMMIO = NULL;
	if ((hMMIO = mmioOpen(file, NULL, MMIO_ALLOCBUF | MMIO_READ)) == NULL) return false;

	MMCKINFO ckparent;
	if (mmioDescend(hMMIO, &ckparent, NULL, 0) != 0) {
		mmioClose(hMMIO, 0);
		return false;
	}
	if ((ckparent.ckid != FOURCC_RIFF) || (ckparent.fccType != mmioFOURCC('W', 'A', 'V', 'E'))) {
		mmioClose(hMMIO, 0);
		return false;
	}

	//	フォーマットチェック
	MMCKINFO ckinfo;
	ckinfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
	mmioDescend(hMMIO, &ckinfo, &ckparent, MMIO_FINDCHUNK);

	PCMWAVEFORMAT	pwf;
	mmioRead(hMMIO, (HPSTR)&pwf, sizeof(pwf));
	if (pwf.wf.wFormatTag != WAVE_FORMAT_PCM) {
		mmioClose(hMMIO, 0);
		return false;
	}

	//	WAVフォーマットの保存
	memcpy(&AudioFormat, &pwf, sizeof(WAVEFORMATEX));

	//	データの読み込み
	mmioSeek(hMMIO, ckparent.dwDataOffset + sizeof(FOURCC), SEEK_SET);
	ckinfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
	mmioDescend(hMMIO, &ckinfo, &ckparent, MMIO_FINDCHUNK);

	MMIOINFO mminfo;
	mmioGetInfo(hMMIO, &mminfo, 0);
	DWORD size = ckinfo.cksize;

	//	バッファ確保
	AudioBufferSize = size;
	BYTE* buf = new BYTE[size];

	//	データの読みとり
	for (DWORD i = 0; i < ckinfo.cksize; i++) {
		if (mminfo.pchNext >= mminfo.pchEndRead) {
			mmioAdvance(hMMIO, &mminfo, MMIO_READ);
			if (mminfo.pchNext >= mminfo.pchEndRead) break;
		}
		*(buf + i) = *((LPBYTE)mminfo.pchNext);
		mminfo.pchNext++;
	}

	AudioBuffer = buf;

	mmioSetInfo(hMMIO, &mminfo, 0);
	mmioClose(hMMIO, 0);
	return true;
}
