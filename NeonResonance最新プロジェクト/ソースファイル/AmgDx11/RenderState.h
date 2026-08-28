#pragma once
#include "DxSystem.h"

class RenderState
{
public:
	static void Initialize();
	static void Release();

	// ブレンドステート
private:
	static ID3D11BlendState* BlendState[3];
public:
	enum BlendType {
		NONE = 0,	// 0:合成無し
		ALPHA,		// 1:通常のアルファブレンド
		ADD,		// 2:加算合成
	};
	static void InitializeBlendState();
	static void SetBlendState(BlendType type);

	// 描画設定
private:
	static ID3D11RasterizerState* Rasterizer[3];
public:
	static void InitializeRasterizer();
	static void SetCullMode(int type);

	// デプス設定
private:
	static ID3D11DepthStencilState* DepthState[3];
public:
	static void InitializeDepthState();
	static void DepthEnable(bool enable, bool write = true);
};
