#include "RenderState.h"
// static変数は宣言必要
ID3D11BlendState* RenderState::BlendState[3];

void RenderState::Release()
{
	for (int i = 0; i < ARRAYSIZE(BlendState); i++) {
		BlendState[i]->Release();
	}

	for (int i = 0; i < ARRAYSIZE(Rasterizer); i++) {
		Rasterizer[i]->Release();
	}

	for (int i = 0; i < ARRAYSIZE(DepthState); i++) {
		DepthState[i]->Release();
	}
}

void RenderState::Initialize()
{
	InitializeDepthState();
	InitializeBlendState();
	InitializeRasterizer();
	SetBlendState(RenderState::ALPHA);
}


void RenderState::InitializeBlendState()
{
	D3D11_BLEND_DESC BlendDesc;
	::ZeroMemory(&BlendDesc, sizeof(BlendDesc));
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	BlendDesc.RenderTarget[0].BlendEnable = false; // ブレンド無し
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	DxSystem::Device->CreateBlendState(&BlendDesc, &BlendState[0]);
	// アルファブレンド
	BlendDesc.RenderTarget[0].BlendEnable = true;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	DxSystem::Device->CreateBlendState(&BlendDesc, &BlendState[1]);
	// 加算ブレンド
	BlendDesc.RenderTarget[0].BlendEnable = true;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	DxSystem::Device->CreateBlendState(&BlendDesc, &BlendState[2]);
}

void RenderState::SetBlendState(BlendType type)
{
	DxSystem::DeviceContext->OMSetBlendState(
		BlendState[type], NULL, 0xFFFFFFFF
	);
}


ID3D11RasterizerState*
RenderState::Rasterizer[3];

void RenderState::InitializeRasterizer()
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID; //D3D11_FILL_WIREFRAME;
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
	DxSystem::Device->CreateRasterizerState(
		&rasterizerDesc, &Rasterizer[0]);

	//表面カリング
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	DxSystem::Device->CreateRasterizerState(
		&rasterizerDesc, &Rasterizer[1]);

	//カリング無し = 両面描画
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	DxSystem::Device->CreateRasterizerState(
		&rasterizerDesc, &Rasterizer[2]);

}

void RenderState::SetCullMode(int type)
{
	DxSystem::DeviceContext->RSSetState(
		Rasterizer[type]);
}


ID3D11DepthStencilState* RenderState::DepthState[3];


void RenderState::InitializeDepthState()
{
	D3D11_DEPTH_STENCIL_DESC desc = {};
	// Z比較ON　Z書き込みON
	desc.DepthEnable = true;
	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.StencilEnable = false;
	DxSystem::Device->CreateDepthStencilState(&desc, &DepthState[0]);

	// Z比較ON　Z書き込みOFF
	desc.DepthEnable = true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	DxSystem::Device->CreateDepthStencilState(&desc, &DepthState[1]);

	// Z比較OFF
	desc.DepthEnable = false;
	DxSystem::Device->CreateDepthStencilState(&desc, &DepthState[2]);
}

void RenderState::DepthEnable(bool enable, bool write)
{
	if (enable) {
		if (write) {
			// Z比較ON　Z書き込みON
			DxSystem::DeviceContext->OMSetDepthStencilState(DepthState[0], 0);
		}
		else {
			// Z比較ON　Z書き込みOFF
			DxSystem::DeviceContext->OMSetDepthStencilState(DepthState[1], 0);
		}
	}
	else {
		// Z比較OFF
		DxSystem::DeviceContext->OMSetDepthStencilState(DepthState[2], 0);
	}
}
