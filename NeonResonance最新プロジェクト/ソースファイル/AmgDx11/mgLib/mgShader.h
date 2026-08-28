#pragma once

#include "../Math/math.h"
#include "../DXShader.h"
#include <DirectXMath.h>

using namespace DirectX;

class mgShader
{
public:

    struct LaneParam
    {
        XMFLOAT4 LaneGlow[5];
    };

    virtual ~mgShader() = default;

    virtual bool Load(
        const wchar_t* filename,
        LPCSTR VSName,
        LPCSTR PSName
    );

    virtual void Bind();

    void SetLaneParam(const LaneParam& param);

    virtual void Update();

    virtual void Release();


protected:

    Shader shader;

    ID3D11Buffer* laneBuffer = nullptr;
};
