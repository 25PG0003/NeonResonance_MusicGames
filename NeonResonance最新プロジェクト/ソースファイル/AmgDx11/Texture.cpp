#include <d3d11.h>
#include "Texture.h"

#if _DEBUG
#pragma comment ( lib, "DirectXTex/DirectXTexD.lib")
#else
#pragma comment ( lib, "DirectXTex/DirectXTex.lib")
#endif

Texture::Texture()
{
}


Texture::~Texture()
{
	ShaderResourceView->Release();
}


bool Texture::Load(const char* filename)
{
	wchar_t	wchar[256];
	size_t wLen = 0;
	errno_t err = 0;

	//変換
	mbstowcs_s(&wLen, wchar, 256, filename, _TRUNCATE);

	// 画像ファイル読み込み DirectXTex
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage image;
	//HRESULT hr = LoadFromWICFile(wchar, 0, &metadata, image);
	HRESULT hr;
	if (_strcmpi(&filename[strlen(filename) - 3], "DDS") == 0) {
		hr = LoadFromDDSFile(wchar, DirectX::DDS_FLAGS_NONE, &metadata, image);
	}
	else if (_strcmpi(&filename[strlen(filename) - 3], "TGA") == 0) {
		hr = LoadFromTGAFile(wchar, &metadata, image);
	}
	else {
		hr = LoadFromWICFile(wchar, DirectX::WIC_FLAGS_NONE, &metadata, image);
	}

	if (FAILED(hr)) {
		return false;
	}

	width = (int)metadata.width;
	height = (int)metadata.height;

	// 画像からシェーダリソースView
	hr = CreateShaderResourceView(DxSystem::Device, image.GetImages(), image.GetImageCount(), metadata, &ShaderResourceView);
	if (FAILED(hr)) {
		return false;
	}
	image.Release();

	return true;
}

bool Texture::LoadMipmap(const char* filename)
{
	wchar_t	wchar[256];
	size_t wLen = 0;
	errno_t err = 0;

	//変換
	mbstowcs_s(&wLen, wchar, 256, filename, _TRUNCATE);

	// 画像ファイル読み込み DirectXTex
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage image;
	//HRESULT hr = LoadFromWICFile(wchar, 0, &metadata, image);
	HRESULT hr;
	if (_strcmpi(&filename[strlen(filename) - 3], "DDS") == 0) {
		hr = LoadFromDDSFile(wchar, DirectX::DDS_FLAGS_NONE, &metadata, image);
	}
	else if (_strcmpi(&filename[strlen(filename) - 3], "TGA") == 0) {
		hr = LoadFromTGAFile(wchar, &metadata, image);
	}
	else {
		hr = LoadFromWICFile(wchar, DirectX::WIC_FLAGS_NONE, &metadata, image);
	}

	if (FAILED(hr)) {
		return false;
	}

	width = (int)metadata.width;
	height = (int)metadata.height;
	// ミップマップ生成
	DirectX::ScratchImage chain;
	DirectX::GenerateMipMaps(
		image.GetImages(),
		image.GetImageCount(),
		image.GetMetadata(),
		DirectX::TEX_FILTER_DEFAULT,
		0, chain);
	// 画像からシェーダリソースView
	hr = CreateShaderResourceView(DxSystem::Device,
		chain.GetImages(), chain.GetImageCount(),
		chain.GetMetadata(), &ShaderResourceView);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

void Texture::Set(int slot)
{
	DxSystem::DeviceContext->PSSetShaderResources(slot, 1, &ShaderResourceView);
	DxSystem::DeviceContext->VSSetShaderResources(slot, 1, &ShaderResourceView);
}

BYTE* Texture::GetPixels()
{
	DirectX::ScratchImage image;
	ID3D11Resource* source;
	ShaderResourceView->GetResource(&source);
	HRESULT hr = CaptureTexture(DxSystem::Device, DxSystem::DeviceContext, source, image);
	uint8_t* ptr = image.GetImage(0, 0, 0)->pixels;

	BYTE* buf = new BYTE[width * height * 4];
	CopyMemory(buf, ptr, width * height * 4);
	return buf;
}

void Texture::HDR_CheckHeader(FILE* fp)
{
	char  buf[256];

	// ヘッダチェック
	for (;;) {
		if (fgets(buf, 256, fp) == NULL) return;
		if (buf[0] == '\n' || buf[0] == '\r') break;

		if (strncmp(buf, "#?", 2) == 0)
		{
			//if (strncmp(buf, "#?RADIANCE", 10) != 0) return -1;
		}
		//	フォーマットチェック
		if (strncmp(buf, "FORMAT=", 7) == 0)
		{
			//if( strncmp( buf, "FORMAT=32-bit_rle_rgbe", 22) != 0 ) return -1;
		}
	}

	// 画像サイズ取得
	if (fgets(buf, 256, fp) == NULL) return;
	char X[5], Y[5];
	sscanf(buf, "%s %d %s %d", Y, &height, X, &width);
}

bool Texture::HDR_ReadLine(BYTE* scanline, FILE* fp)
{
	//	ライン情報読み込み
	int val0 = getc(fp);	// 0x02
	int val1 = getc(fp);	// 0x02
	int val2 = getc(fp);	// size
	int val3 = getc(fp);	// size
	// 幅チェック
	if ((val2 << 8 | val3) != width) return false;

	for (int ch = 0; ch < 4; ch++)
	{
		for (int x = 0; x < width; )
		{
			int length = getc(fp);
			if (length == EOF) return false;

			bool Runlength = false;
			int val = 0;
			if (length > 128) {
				// ランレングス設定
				Runlength = true;
				length -= 128;
				val = getc(fp);
			}
			//	データ読み込み
			while (length > 0) {
				if (Runlength) scanline[x * 4 + ch] = val;
				else scanline[x * 4 + ch] = getc(fp);
				x++;
				length--;
			}
		}
	}
	return true;
}

bool Texture::HDR_ReadPixels(FILE* fp, float* buf)
{
	int ret = 0;
	BYTE scanin[8192 * 4];
	float* work = buf;

	for (int y = height - 1; y >= 0; y--) {
		if (!HDR_ReadLine(scanin, fp)) return false;
		for (int x = 0; x < width; x++) {
			float exp = powf(2, scanin[x * 4 + 3] - 128.0f);
			work[0] = scanin[x * 4 + 0] * exp;
			work[1] = scanin[x * 4 + 1] * exp;
			work[2] = scanin[x * 4 + 2] * exp;
			work[3] = 1;
			work += 4;
		}
	}
	return true;
}

bool Texture::LoadHDR(const char* filename)
{
	HRESULT hr = S_OK;

	FILE* fd;

	if ((fd = fopen(filename, "rb")) == NULL)
	{
		return false;
	}

	// ヘッダーを調べる
	HDR_CheckHeader(fd);

	// コンバートする
	float* buf = new float[width * height * 4];
	HDR_ReadPixels(fd, buf);
	fclose(fd);

	DirectX::Image img;
	img.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	img.width = width;
	img.height = height;
	img.rowPitch = sizeof(float) * 4 * img.width;
	img.slicePitch = img.rowPitch * img.height;
	img.pixels = (uint8_t*)buf;

	DirectX::ScratchImage image;
	image.InitializeFromImage(img);

	DirectX::ScratchImage mipChain;
	DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain);

	// 画像からシェーダリソースView
	CreateShaderResourceView(DxSystem::Device, mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), &ShaderResourceView);

	return true;
}

