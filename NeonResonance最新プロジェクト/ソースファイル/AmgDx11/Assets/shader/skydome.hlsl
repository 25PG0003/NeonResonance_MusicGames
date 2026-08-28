//Assets/shader/skydome.hlsl
// プログラム側から受け取る情報
cbuffer CBBaseMatrix : register(b0)
{
    matrix World;
    matrix View;
    matrix Proj;
    matrix matWVP;
};


//	テクスチャ
Texture2D Environment : register(t15); // 環境光テクスチャ

// テクスチャサンプラー
SamplerState WrapSampler : register(s0);

//	入力頂点
struct APPtoVS
{
    float3 Position : POSITION;
    float2 Tex : TEXCOORD;
};

struct VStoPS
{
    float4 Position : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------
//	頂点シェーダー
//--------------------------------------------
VStoPS VSMain(APPtoVS input)
{
    VStoPS output = (VStoPS) 0;

    float4 P = float4(input.Position, 1.0);
    output.Position = mul(matWVP, P);
    output.Tex = input.Tex;

    return output;
}

//--------------------------------------------
//	ピクセルシェーダー
//--------------------------------------------
float4 PSMain(VStoPS input) : SV_TARGET0
{
    float4 tex = Environment.Sample(WrapSampler, input.Tex);
	// トーンマップ
    tex.rgb = tex.rgb / (tex.rgb + 40); //数値は調整値
    
    float4 color = tex;
    return color;
}

