#pragma once

#include <d3d11.h>
#include "Math/math.h"

class DxSystem {
public:
	static int ScreenWidth;
	static int ScreenHeight;
	static ID3D11RenderTargetView* RenderTargetView;
	static ID3D11DepthStencilView* DepthStencilView;

	// Preview用
	static ID3D11Texture2D* PreviewTexture;
	static ID3D11RenderTargetView* PreviewRenderTargetView;
	static ID3D11ShaderResourceView* PreviewShaderResourceView;

	static ID3D11Texture2D* PreviewDepthTexture;
	static ID3D11DepthStencilView* PreviewDepthStencilView;

	static bool InitializePreviewRenderTarget(int width, int height);
	static void BeginPreviewRender();
	static void EndPreviewRender();

private:
	static IDXGISwapChain*			SwapChain;

	static ID3D11Texture2D*				DepthStencilTexture;
	static ID3D11ShaderResourceView*	ShaderResourceView;

	static HRESULT CreateDevice(HWND hWnd);
	static bool CreateDepthStencil();
	static bool InitializeRenderTarget();

public:
	static ID3D11Device*			Device;
	static ID3D11DeviceContext*		DeviceContext;

	static bool Initialize(HWND hWnd, int width, int height);
	static void Release();
	static void Clear(float r = 0.431f, float g = 0.588f, float b = 0.729f);
	static void Flip();

	static Matrix ViewMatrix;
	static Matrix ProjectionMatrix;
};
