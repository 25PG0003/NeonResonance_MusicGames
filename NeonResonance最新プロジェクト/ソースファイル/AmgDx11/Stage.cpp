#include "Stage.h"
#include "DXShader.h"
#include "imgui/imgui.h"
#include "GameSystem.h"
#include "JudgeLine.h"
#include "DxSystem.h"
#include "DXShader.h"

Stage::Stage() : scrollspeed(1.0f)
{
   
}

Stage::~Stage()
{
    delete stageMesh;
    delete judgeline;
}

bool Stage::Initialize()
{
    stageMesh = new FBXMesh();
    gametime = new GameTime();
    judgeline = new FBXMesh();

    // Camera初期化（位置取得用）
    camera.Initialize(DxSystem::ScreenWidth, DxSystem::ScreenHeight);

   

    stageMesh->Create("Assets/stage/main_stage.fbx");
    stageMesh->position = Vector3(0.0f, 0.0f , -80.0f);
    stageMesh->scale = Vector3(0.5f, 0.0f, 200.0f);
    stageMesh->rotation = Vector3(0.0f, 0.0f, 0.0f);
    judgeline->Create("Assets/music_Assets/judgeline.fbx");
    judgeline->scale = Vector3(5.0f,0.02f,0.01f);

    laneShader.Load(L"Assets/shader/Glow.hlsl", "VSMain", "PSMain");

    OutputDebugStringA("Load End\n");

    return true;
}


void Stage::Update(GameTime& time)
{
    // =========================
   // DxSystem側View/Projection（既存維持）
   // =========================
    DxSystem::ViewMatrix.LookAt(
        Vector3(0, Config::LaneAngle, 3),  //6～13あたり
        Vector3(0, 1, 0)
    );

    DxSystem::ProjectionMatrix.PerspectiveFov(
        1.0f,
        (float)DxSystem::ScreenWidth / DxSystem::ScreenHeight,
        0.1f,
        1500.0f
    );

    stageMesh->Update();
    judgeline->Update();
}

void Stage::Draw()
{

    if (!stageMesh || !judgeline) return;

    //Lane情報取得
    if (player)
    {
        const auto& lane = player->GetLaneParam();

        mgShader::LaneParam param = {};


        for (int i = 0; i < 5; i++) {
            param.LaneGlow[i] = player->GetLaneParam().GlowColor[i];
        }
        laneShader.SetLaneParam(param);
    }

    // ---- Stage ----
    Matrix stageWorld;
    stageWorld.TRS(
        stageMesh->position,
        stageMesh->rotation,
        stageMesh->scale
    );

    stageMesh->transform = stageWorld;
    Shader::SetTransform(stageMesh->transform);
    laneShader.Bind();
    stageMesh->Render();

    // ---- JudgeLine ----
    Matrix judgeWorld;
    judgeWorld.TRS(
        Vector3(0.0f, 0.1, 1.5),   // 判定ライン固定位置
        Vector3(0.0f, 0.0f, 0.0f),
        judgeline->scale
    );

    judgeline->transform = judgeWorld;
    Shader::SetTransform(judgeline->transform);
    judgeline->Render();

    
}

void Stage::SetPlayer(Player* p)
{
    player = p;
}

#if _DEBUG
void Stage::DebugInfomation()
{
    ImGui::Begin("Stage");
    ImGui::Text("scrollspeed:%f", scrollspeed);
    ImGui::End();
}
#endif
