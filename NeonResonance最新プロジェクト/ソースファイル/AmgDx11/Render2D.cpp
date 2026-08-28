#include "DxSystem.h"
#include "Render2D.h"

// いらないもの消す

Render2D::Render2D()
{
}

Render2D::~Render2D()
{
	if (vertices) { delete[] vertices; vertices = nullptr; }
	if (texture) { delete texture; texture = nullptr; }
	if (shader) { delete shader; shader = nullptr; }

	if (VertexBuffer) { VertexBuffer->Release(); VertexBuffer = nullptr; }
	if (IndexBuffer) { IndexBuffer->Release(); IndexBuffer = nullptr; }
}

void Render2D::Initialize(const char* filename, Shader* shader)
{
	// ファイル名の指定がないときはテクスチャは読み込まない
	if (filename) {
		texture = new Texture();
		if (!texture->Load(filename)) {
			return;
		}
	}
	else {
		texture = nullptr;
	}
	// 使用シェーダ読み込み
	if (shader) {
		this->shader = shader;
	}
	else {
		this->shader = new Shader();
		// テクスチャの有無を確認
		if (!texture) {
			// テクスチャがないとき
			this->shader->Create(L"Assets/shader/2D.fx", "VSMain", "PSBase");
		}
		else {
			// テクスチャがあるとき
			this->shader->Create(L"Assets/shader/2D.fx", "VSMain", "PSTex");
		}
	}

	//	頂点初期化
	vertices = new Vertex[4];
	for (int i = 0; i < 4; i++) {
		vertices[i].r = 1;
		vertices[i].g = 1;
		vertices[i].b = 1;
		vertices[i].a = 1;
	}

	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(Vertex) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		// サブリソースの設定
		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
		initData.pSysMem = vertices;
		HRESULT hr = DxSystem::Device->CreateBuffer(&bd, &initData, &VertexBuffer);
	}

	{
		int* indices = new int[6]();
		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 1;
		indices[4] = 3;
		indices[5] = 2;
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(int) * 6;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		// サブリソースの設定
		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
		initData.pSysMem = indices;
		HRESULT hr = DxSystem::Device->CreateBuffer(&bd, &initData, &IndexBuffer);
		delete[] indices;
	}

	float sx = texture != nullptr ? (float)texture->GetWidth() : 64;
	float sy = texture != nullptr ? (float)texture->GetHeight() : 64;

	position = Vector3(0, 0, 0);
	rotation = 0;
	width = sx;
	height = sy;
	u = v = 0;
	w = h = 1;
	r = g = b = a = 1;
	cx = cy = 0;
}

////--------------------------------------------------
//// 解放
////--------------------------------------------------
//void Render2D::Release()
//{
//	delete vertices;
//}

//--------------------------------------------------
// 描画
//--------------------------------------------------
void Render2D::Render()
{
	UpdateVertices();

	shader->Activate();
	if (texture) {
		texture->Set();
	}

	//// 行列設定
	//DxSystem::DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	//DxSystem::DeviceContext->PSSetConstantBuffers(0, 1, &ConstantBuffer);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DxSystem::DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
	DxSystem::DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DxSystem::DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//	描画
	DxSystem::DeviceContext->DrawIndexed(6, 0, 0);


	ID3D11Buffer* nullVB = nullptr;
	UINT zero = 0;
	DxSystem::DeviceContext->IASetVertexBuffers(0, 1, &nullVB, &zero, &zero);
}

void Render2D::SetPositon(const Vector3& pos)
{
	position = pos;
}

void Render2D::SetSize(float width, float height)
{
	this->width = width;
	this->height = height;
}

void Render2D::SetUV(float u, float v, float w, float h)
{
	this->u = u;
	this->v = v;
	this->w = w;
	this->h = h;
}

void Render2D::SetColor(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

void Render2D::SetRotation(float rad)
{
	rotation = rad;
}

void Render2D::SetOrigin(float cx, float cy)
{
	this->cx = cx; this->cy = cy;
}

//	全頂点更新
void Render2D::UpdateVertices()
{
	vertices[0].x = -cx;
	vertices[0].y = -cy;
	vertices[0].tu = u;
	vertices[0].tv = v;

	vertices[1].x = -cx;
	vertices[1].y = -cy + height;
	vertices[1].tu = u;
	vertices[1].tv = v + h;

	vertices[2].x = -cx + width;
	vertices[2].y = -cy;
	vertices[2].tu = u + w;
	vertices[2].tv = v;

	vertices[3].x = -cx + width;
	vertices[3].y = -cy + height;
	vertices[3].tu = u + w;
	vertices[3].tv = v + h;

	float fsin = sinf(rotation);
	float fcos = cosf(rotation);

	for (int i = 0; i < 4; i++) {
		// 回転後の頂点座標を求める
		float dx = vertices[i].x * fcos - vertices[i].y * fsin;
		float dy = vertices[i].x * fsin + vertices[i].y * fcos;

		vertices[i].x = dx;
		vertices[i].y = dy;

		vertices[i].x += position.x;
		vertices[i].y += position.y;

		// veritces.x →  0 ～ 1920
		//            → -1 ～ 1
		// vertices.y →  0 ～ 1080
		//            → -1 ～ 1
		// 座標変換
		// 1920を1にする計算
		// 1080を1にする計算
		vertices[i].x /= DxSystem::ScreenWidth;
		vertices[i].y /= DxSystem::ScreenHeight;
		//  0～1 1

		// -1～1 2
		vertices[i].x = vertices[i].x * 2 - 1;
		vertices[i].y = vertices[i].y * 2 - 1;
		vertices[i].y *= -1;

		vertices[i].r = r;
		vertices[i].g = g;
		vertices[i].b = b;
		vertices[i].a = a;
	}

	vertices[0].z = vertices[1].z = vertices[2].z = vertices[3].z = position.z;

	DxSystem::DeviceContext->UpdateSubresource(VertexBuffer, 0, NULL, vertices, 0, 0);
}

void Render2D::SetSpriteSheetUV(int index, int cols, int rows)
{
	float cellW = 1.0f / cols;
	float cellH = 1.0f / rows;

	int x = index % cols;
	int y = index / cols;

	float u = x * cellW;
	float v = y * cellH;

	SetUV(u, v, cellW, cellH);
}

//void Render2D::Initialize()
//{
//	assert(baseShader.Create(L"Assets/2D.fx", "VSMain", "PSMain"));
//
//	// 定数バッファ生成
//	D3D11_BUFFER_DESC bd;
//	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
//	bd.ByteWidth = sizeof(CBParam);
//	bd.Usage = D3D11_USAGE_DEFAULT;
//	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//	bd.CPUAccessFlags = 0;
//
//	Matrix matrix;
//	Vector3 p = Vector3(-1, 1, 0);
//	Vector3 r = Vector3(0, 0, 0);
//	Vector3 s = Vector3(1, -1, 1);
//	matrix.TRS(p, r, s);
//	CBParam param;
//	param.proj.Ortho(1280, 720, 0, 1000);
//	param.proj *= matrix;
//
//	// サブリソースの設定.
//	D3D11_SUBRESOURCE_DATA initData;
//	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
//	initData.pSysMem = &param;
//
//	DxSystem::Device->CreateBuffer(&bd, &initData, &ConstantBuffer);
//}

//Shader Render2D::baseShader;
//ID3D11Buffer* Render2D::ConstantBuffer;