#include "JudgeLine.h"
#include "DXShader.h"
#include "GameSystem.h"

JudgeLine::JudgeLine()
{
    // 位置（プレイヤーの足元あたり）
    position = Vector3(0.0f, -1.0f, 0.0f);

    // プレイヤーの幅に合わせる
    scale = Vector3(3.0f, 0.05f, 1.0f);

    mesh = nullptr;
}

JudgeLine::~JudgeLine()
{
    if (mesh)
    {
        delete mesh;
        mesh = nullptr;
    }
}

void JudgeLine::Initialize()
{
    mesh = new FBXMesh();

    // FBXモデル読み込み（薄いPlane1枚）
    mesh->Create("Assets/judge/judge_line.fbx");

    // 大きさ・位置調整
    mesh->scale = scale;
    mesh->position = position;
}

void JudgeLine::Update()
{
    // 必要であればアニメ処理など　
    mesh->position = position;
    mesh->scale = scale;

    mesh->Update();
}

void JudgeLine::Draw()
{
    Shader::SetTransform(mesh->transform);
    mesh->Render();
}
