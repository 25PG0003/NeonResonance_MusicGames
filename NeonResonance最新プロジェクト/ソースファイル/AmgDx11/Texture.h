#pragma once
#include "DxSystem.h"
#include "DirectXTex\DirectXTex.h"

class Texture
{
private:
	ID3D11ShaderResourceView* ShaderResourceView;
	int width;
	int height;

	void HDR_CheckHeader(FILE* fp);
	bool HDR_ReadLine(BYTE* scanline, FILE* fp);
	bool HDR_ReadPixels(FILE* fp, float* buf);


public:
	Texture();
	~Texture();

	bool Load(const char* filename);
	bool LoadMipmap(const char* filename);
	bool LoadHDR(const char* filename);
	void Set(int slot = 0);

	int GetWidth() { return width; }
	int GetHeight() { return height; }

	BYTE* GetPixels();
};
