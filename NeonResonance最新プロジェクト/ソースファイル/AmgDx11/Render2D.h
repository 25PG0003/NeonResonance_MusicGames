#pragma once
#include <d3d11.h>
#include "Math/math.h"
#include "Texture.h"
#include "DxShader.h"


class Render2D
{
public:
	Render2D();
	~Render2D();

	void Initialize(const char* filename = nullptr, Shader* shader = nullptr);
	//void Release();
	void Render();

public:
	void SetPositon(const Vector3& pos);
	void SetSize(float width, float height);
	void SetUV(float u, float v, float w, float h);
	void SetColor(float r, float g, float b, float a = 1.0f);
	void SetRotation(float rad);
	void SetOrigin(float cx, float cy);
	void SetSpriteSheetUV(int index, int cols, int rows);

protected:
	void UpdateVertices();

public:
	//static void Initialize();

protected:
	/*struct CBParam {
		Matrix proj;
	};*/
	//	描画用情報
	//	頂点構造体（シェーダーと一致）
	struct Vertex {
		float x, y, z;		// 座標
		float nx, ny, nz;	// 法線
		float tu, tv;		// UV
		float r, g, b, a;	// 頂点カラー
	};
	Vector3 position;
	float width, height;
	float u, v, w, h;
	float r, g, b, a;
	float rotation;
	float cx, cy;

	Shader* shader;
	Texture* texture;

	Vertex* vertices;

	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;

	//static Shader baseShader;
	//static ID3D11Buffer* ConstantBuffer;
};