#include "DxSystem.h"
#include "DXShader.h"

//****************************************************************
//
//	シェーダーシステム初期化・解放
//
//****************************************************************
ID3D11SamplerState* Shader::SamplerStateWrap;
ID3D11SamplerState* Shader::SamplerStateClamp;
ID3D11SamplerState* Shader::SamplerStateBorder;
ID3D11Buffer* Shader::CBBaseMatrix;

bool Shader::InitializeSystem()
{
	HRESULT hr;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Slot0 Wrap
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = DxSystem::Device->CreateSamplerState(&sampDesc, &SamplerStateWrap);
	if (FAILED(hr)) return false;
	DxSystem::DeviceContext->PSSetSamplers(0, 1, &SamplerStateWrap);
	DxSystem::DeviceContext->VSSetSamplers(0, 1, &SamplerStateWrap);

	// Slot1 Clamp
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	hr = DxSystem::Device->CreateSamplerState(&sampDesc, &SamplerStateClamp);
	if (FAILED(hr)) return false;
	DxSystem::DeviceContext->PSSetSamplers(1, 1, &SamplerStateClamp);
	DxSystem::DeviceContext->VSSetSamplers(1, 1, &SamplerStateClamp);

	// Slot2 Border
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.BorderColor[0] = D3D11_FLOAT32_MAX;
	sampDesc.BorderColor[1] = D3D11_FLOAT32_MAX;
	sampDesc.BorderColor[2] = D3D11_FLOAT32_MAX;
	sampDesc.BorderColor[3] = D3D11_FLOAT32_MAX;
	hr = DxSystem::Device->CreateSamplerState(&sampDesc, &SamplerStateBorder);
	if (FAILED(hr)) return false;
	DxSystem::DeviceContext->PSSetSamplers(2, 1, &SamplerStateBorder);
	DxSystem::DeviceContext->VSSetSamplers(2, 1, &SamplerStateBorder);


	// 基本コンスタントバッファ生成
	CBBaseMatrix = CreateConstantBuffer(sizeof(CBBaseMatrixParam));

	return true;
}

void Shader::ReleaseSystem()
{
	SamplerStateWrap->Release();
	SamplerStateClamp->Release();
	SamplerStateBorder->Release();
	CBBaseMatrix->Release();
}

ID3D11Buffer* Shader::CreateConstantBuffer( UINT size)
{
	ID3D11Buffer* cb;
	// 定数バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.ByteWidth = size;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	DxSystem::Device->CreateBuffer(&bd, NULL, &cb);

	return cb;
}

//****************************************************************
//
//
//
//****************************************************************

Shader::Shader()
{
	VS = nullptr;
	PS = nullptr;
	VertexLayout = nullptr;
}

Shader::~Shader()
{
	if (VertexLayout) { VertexLayout->Release(); VertexLayout = NULL; }
	if (VS) { VS->Release(); VS = NULL; }
	if (PS) { PS->Release(); PS = NULL; }
}
//------------------------------------------------
//	シェーダー単体コンパイル
//------------------------------------------------
HRESULT Shader::Compile(WCHAR* filename, LPCSTR method, LPCSTR shaderModel, ID3DBlob** blobOut)
{
	DWORD ShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	//ShaderFlags |= D3DCOMPILE_DEBUG;
	//ShaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;

	ID3DBlob* BlobError = NULL;
	// コンパイル
	HRESULT hr = D3DCompileFromFile(
		filename,
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		method,
		shaderModel,
		ShaderFlags,
		0,
		blobOut,
		&BlobError
	);

	// エラー出力
	if (BlobError != NULL)
	{
		OutputDebugStringA("----------------------------\n");
		OutputDebugStringA((char*)BlobError->GetBufferPointer());
		OutputDebugStringA("----------------------------\n");
		BlobError->Release();
		BlobError = NULL;
	}

	return hr;
}


Shader::CBBaseMatrixParam Shader::MatrixParam;
void Shader::SetTransform(Matrix& transform)
{
	MatrixParam.view = DxSystem::ViewMatrix;
	MatrixParam.proj = DxSystem::ProjectionMatrix;
	Matrix MatVP = MatrixParam.view * MatrixParam.proj;

	MatrixParam.world = transform;
	MatrixParam.wvp = MatrixParam.world * MatVP;
	DxSystem::DeviceContext->UpdateSubresource(Shader::CBBaseMatrix, 0, NULL, &MatrixParam, 0, 0);

	Matrix inv = DxSystem::ViewMatrix;
	inv.inverse(); //カメラ(逆行列)を元(逆の逆)に戻す
	MatrixParam.viewPosition = Vector3(inv._41, inv._42, inv._43);

}

//------------------------------------------------
//	シェーダーセットコンパイル
//------------------------------------------------
bool Shader::Create(WCHAR* filename, LPCSTR VSFunc, LPCSTR PSFunc )
{
	HRESULT hr = S_OK;

	ID3DBlob* VSBlob = NULL;
	// 頂点シェーダ
	hr = Compile(filename, VSFunc, "vs_5_0", &VSBlob);
	if (FAILED(hr))
	{
		return false;
	}

	// 頂点シェーダ生成
	hr = DxSystem::Device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), NULL, &VS);
	if (FAILED(hr))
	{
		VSBlob->Release();
		return false;
	}

	// 入力レイアウト
	D3D11_INPUT_ELEMENT_DESC layout3D[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4*3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4*6, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
						4*12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
						4 * 15, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout3D);

	// 入力レイアウト生成
	hr = DxSystem::Device->CreateInputLayout(
		layout3D,
		numElements,
		VSBlob->GetBufferPointer(),
		VSBlob->GetBufferSize(),
		&VertexLayout
	);

	VSBlob->Release();
	if (FAILED(hr))
	{
		return false;
	}

	// 入力レイアウト設定
	DxSystem::DeviceContext->IASetInputLayout(VertexLayout);


	// ピクセルシェーダ
	ID3DBlob* PSBlob = NULL;
	hr = Compile(filename, PSFunc, "ps_5_0", &PSBlob);
	if (FAILED(hr))
	{
		return false;
	}

	// ピクセルシェーダ生成
	hr = DxSystem::Device->CreatePixelShader( PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), NULL, &PS );
	PSBlob->Release();
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

//****************************************************************
//
//
//
//****************************************************************
//------------------------------------------------
//	有効化
//------------------------------------------------
void Shader::Activate()
{
	// 入力レイアウト設定
	DxSystem::DeviceContext->IASetInputLayout(VertexLayout);
	DxSystem::DeviceContext->VSSetShader(VS, NULL, 0);
	DxSystem::DeviceContext->PSSetShader(PS, NULL, 0);
	DxSystem::DeviceContext->GSSetShader(NULL, NULL, 0);
}

