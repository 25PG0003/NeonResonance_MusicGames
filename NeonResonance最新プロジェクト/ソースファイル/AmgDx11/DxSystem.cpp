#include "DxSystem.h"

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

ID3D11Device*			DxSystem::Device;
IDXGISwapChain*			DxSystem::SwapChain;
ID3D11DeviceContext*	DxSystem::DeviceContext;
ID3D11RenderTargetView* DxSystem::RenderTargetView;

ID3D11Texture2D*            DxSystem::DepthStencilTexture;
ID3D11DepthStencilView*     DxSystem::DepthStencilView;
ID3D11ShaderResourceView*   DxSystem::ShaderResourceView;

ID3D11Texture2D* DxSystem::PreviewTexture = nullptr;
ID3D11RenderTargetView* DxSystem::PreviewRenderTargetView = nullptr;
ID3D11ShaderResourceView* DxSystem::PreviewShaderResourceView = nullptr;

ID3D11Texture2D* DxSystem::PreviewDepthTexture = nullptr;
ID3D11DepthStencilView* DxSystem::PreviewDepthStencilView = nullptr;

int DxSystem::ScreenWidth = 1920;
int DxSystem::ScreenHeight = 1080;

Matrix DxSystem::ViewMatrix;
Matrix DxSystem::ProjectionMatrix;

//****************************************************************
//
//	初期化
//
//****************************************************************
bool DxSystem::Initialize(HWND hWnd, int width, int height)
{
	CreateDevice(hWnd);
	InitializeRenderTarget();

	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;

	ID3D11RasterizerState * pState;
	Device->CreateRasterizerState(&rasterizerDesc, &pState);
	DeviceContext->RSSetState(pState);
	pState->Release();

	InitializePreviewRenderTarget(800, 450);
	return true;
}

//****************************************************************
//
//	デバイス生成
//
//****************************************************************
HRESULT DxSystem::CreateDevice( HWND hWnd )
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

	// 機能レベル
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = sizeof(featureLevels) / sizeof(featureLevels[0]);

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_10_0;

	// スワップチェインの設定
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = ScreenWidth;
	sd.BufferDesc.Height = ScreenHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;   // ★重要
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// デバイス生成
	hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		featureLevels,
		numFeatureLevels,
		D3D11_SDK_VERSION,
		&sd,
		&SwapChain,
		&Device,
		&featureLevel,
		&DeviceContext);
	
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

void DxSystem::Release()
{
	if (DeviceContext) { DeviceContext->ClearState(); }
	if (RenderTargetView) { RenderTargetView->Release(); RenderTargetView = NULL; }
	if (SwapChain) { SwapChain->Release(); SwapChain = NULL; }
	if (DeviceContext) { DeviceContext->Release(); DeviceContext = NULL; }
	if (Device) { Device->Release(); Device = NULL; }
}

//****************************************************************
//
//	レンダーターゲット関連
//
//****************************************************************
//------------------------------------------------
//	初期化
//------------------------------------------------
bool DxSystem::InitializeRenderTarget()
{
	// バックバッファ取得
	ID3D11Texture2D* BackBuffer = NULL;
	HRESULT hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	// レンダーターゲットビュー生成
	hr = Device->CreateRenderTargetView(BackBuffer, NULL, &RenderTargetView);
	BackBuffer->Release();
	BackBuffer = NULL;
	if (FAILED(hr))
	{
		return false;
	}

	CreateDepthStencil();

	// レンダーターゲットビュー設定
	DeviceContext->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);

	// ビューポート設定
	D3D11_VIEWPORT vp;
	vp.Width = (float)ScreenWidth;
	vp.Height = (float)ScreenHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	DeviceContext->RSSetViewports(1, &vp);

	return true;
}

//------------------------------------------------
//      深度ステンシルバッファ生成
//------------------------------------------------
bool DxSystem::CreateDepthStencil()
{
	// 深度ステンシル設定
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(D3D11_TEXTURE2D_DESC));
	td.Width = ScreenWidth;
	td.Height = ScreenHeight;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R24G8_TYPELESS;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;

	// 深度ステンシルテクスチャ生成
	HRESULT hr = Device->CreateTexture2D(&td, NULL, &DepthStencilTexture);
	if (FAILED(hr))
	{
		return false;
	}

	// 深度ステンシルビュー設定
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;

	// 深度ステンシルビュー生成
	hr = Device->CreateDepthStencilView(DepthStencilTexture, &dsvd, &DepthStencilView);
	if (FAILED(hr))
	{
		return false;
	}

	// シェーダリソースビュー設定
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvd.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = 1;

	// シェーダリソースビュー生成
	hr = Device->CreateShaderResourceView(DepthStencilTexture, &srvd, &ShaderResourceView);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool DxSystem::InitializePreviewRenderTarget(int width, int height)
{
	//==================================================
	// カラーテクスチャ
	//==================================================

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	textureDesc.BindFlags =
		D3D11_BIND_RENDER_TARGET |
		D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = Device->CreateTexture2D(
		&textureDesc,
		nullptr,
		&PreviewTexture
	);

	if (FAILED(hr))
		return false;


	//==================================================
	// RenderTargetView
	//==================================================

	hr = Device->CreateRenderTargetView(
		PreviewTexture,
		nullptr,
		&PreviewRenderTargetView
	);

	if (FAILED(hr))
		return false;


	//==================================================
	// ShaderResourceView
	//==================================================

	hr = Device->CreateShaderResourceView(
		PreviewTexture,
		nullptr,
		&PreviewShaderResourceView
	);

	if (FAILED(hr))
		return false;


	//==================================================
	// DepthStencil
	//==================================================

	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = Device->CreateTexture2D(
		&depthDesc,
		nullptr,
		&PreviewDepthTexture
	);

	if (FAILED(hr))
		return false;


	//==================================================
	// DepthStencilView
	//==================================================

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = Device->CreateDepthStencilView(
		PreviewDepthTexture,
		&dsvDesc,
		&PreviewDepthStencilView
	);

	if (FAILED(hr))
		return false;


	return true;
}

void DxSystem::BeginPreviewRender()
{
	ID3D11RenderTargetView* rtv = PreviewRenderTargetView;

	DeviceContext->OMSetRenderTargets(
		1,
		&rtv,
		PreviewDepthStencilView
	);

	float clearColor[4] =
	{
		0.0f,
		0.0f,
		0.0f,
		1.0f
	};

	DeviceContext->ClearRenderTargetView(
		PreviewRenderTargetView,
		clearColor
	);

	DeviceContext->ClearDepthStencilView(
		PreviewDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0
	);

	D3D11_VIEWPORT viewport{};

	viewport.Width = 800.0f;
	viewport.Height = 450.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	DeviceContext->RSSetViewports(
		1,
		&viewport
	);
}

void DxSystem::EndPreviewRender()
{
	DeviceContext->OMSetRenderTargets(
		1,
		&RenderTargetView,
		DepthStencilView
	);

	D3D11_VIEWPORT viewport{};

	viewport.Width = (float)ScreenWidth;
	viewport.Height = (float)ScreenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	DeviceContext->RSSetViewports(
		1,
		&viewport
	);
}


//------------------------------------------------
//	クリア
//------------------------------------------------
void DxSystem::Clear(float r, float g, float b)
{
	float clearColor[4] = { r, g, b, 1.0f };
	DeviceContext->ClearRenderTargetView(RenderTargetView, clearColor);
	DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

//------------------------------------------------
//	フリップ
//------------------------------------------------
void DxSystem::Flip()
{
	// フリップ処理
	SwapChain->Present(1, 0);
}
