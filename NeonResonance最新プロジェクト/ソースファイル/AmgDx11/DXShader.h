#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>

struct float4 {
	union {
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
		float v[4];
	};
};

class Shader
{
public:
	static bool InitializeSystem();
	static void ReleaseSystem();

	static ID3D11Buffer* CreateConstantBuffer(UINT size);

	// モデル用コンスタントバッファ
	struct CBBaseMatrixParam {
		Matrix world;
		Matrix view;
		Matrix proj;
		Matrix wvp;

		Vector3 viewPosition; // カメラ位置
		float dummy;
	};
	static ID3D11Buffer* CBBaseMatrix;

	static ID3D11SamplerState* SamplerStateWrap;
	static ID3D11SamplerState* SamplerStateClamp;
	static ID3D11SamplerState* SamplerStateBorder;
	
public:
	Shader();
	~Shader();

	bool Create(WCHAR* filename, LPCSTR VSName, LPCSTR PSName);
	void Activate();

protected:
	ID3D11VertexShader* VS; // 頂点シェーダ
	ID3D11PixelShader* PS; // ピクセルシェーダ
	ID3D11InputLayout* VertexLayout;

	HRESULT Shader::Compile(WCHAR* filename, LPCSTR method, LPCSTR shaderModel, ID3DBlob** ppBlobOut);

public:
	static CBBaseMatrixParam MatrixParam;
	static void SetTransform(Matrix& transform);
};

