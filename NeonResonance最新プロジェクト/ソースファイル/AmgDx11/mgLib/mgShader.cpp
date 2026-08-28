#include "mgShader.h"
#include "../DxSystem.h"

bool mgShader::Load(const wchar_t* filename, LPCSTR VSName, LPCSTR PSName)
{
	return shader.Create(const_cast<WCHAR*>(filename), VSName, PSName);
}

void mgShader::SetLaneParam(const LaneParam& param)
{
	if (!laneBuffer)
	{
		D3D11_BUFFER_DESC bd = {};

		bd.ByteWidth = sizeof(LaneParam);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		DxSystem::Device->CreateBuffer(&bd, nullptr, &laneBuffer);

	}

	DxSystem::DeviceContext->UpdateSubresource(
		laneBuffer, 0, nullptr,
		&param, 0, 0
	);

	DxSystem::DeviceContext->PSSetConstantBuffers(
		1, 1, &laneBuffer
	);
}

void mgShader::Bind()
{
	shader.Activate();
}

void mgShader::Update()
{
}

void mgShader::Release()
{
}
