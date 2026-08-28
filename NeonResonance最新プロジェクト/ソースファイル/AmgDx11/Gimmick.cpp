#include "Gimmick.h"
#include "DXShader.h"
#include <algorithm>
#include <vector>
#include <fstream>
#include <sstream>
#include "imgui/imgui.h"
#include <iostream>
#include "Render2D.h"
#include "Input/Input.h"
#include "RenderState.h"
#include <cmath>
#include "SceneTest.h"
#include "AudioSystem/AudioSource.h"

#include <filesystem>




static const float verticalLaneX[6] = {
    -2.5f,  // 左端
    -1.5f,
    -0.5f,
     0.5f,
     1.5f,
     2.5f  
};

Gimmick::Gimmick()
{
}

Gimmick::~Gimmick()
{
}



//--------------------------------------
// 初期化
//--------------------------------------
void Gimmick::Initialize()
{
    

    Perfect.Initialize("Assets/judge/perfect.png");
    Perfect.SetSize(100, 100);
    Perfect.SetPositon(Vector3(870, 750, 0));

    Good.Initialize("Assets/judge/Good.png");
    Good.SetSize(100, 100);
    Good.SetPositon(Vector3(870, 750, 0));

    Miss.Initialize("Assets/judge/miss.png");
    Miss.SetSize(100, 100);
    Miss.SetPositon(Vector3(870, 750, 0));

    ComboNum.Initialize("Assets/judge/number80.png");
    ComboNum.SetSize(40, 80);
    ComboNum.SetUV(0.0f, 0.0f, 0.1f, 1.0f);
    ComboNum.SetPositon(Vector3(580, 100, 0));

    perfectEffect.Initialize("Assets/effect/effect_1.png");
    perfectEffect.SetSize(200, 200);

    goodEffect.Initialize("Assets/effect/effect_2.png");
    goodEffect.SetSize(200, 200);

    missEffect.Initialize("Assets/effect/effect_3.png");
    missEffect.SetSize(200, 200);

    judgeLineConfig.color = Vector3(1.0f, 1.0f, 0.0f);
    judgeLineConfig.alpha = 1.0f;

    notedata.Initialize();
}


void Gimmick::SetSpeed(float speed)
{
    notedata.SetSpeed(speed);
    
}

void Gimmick::Release()
{
    notedata.Release();
}

void Gimmick::ClearRelease()
{
    notedata.ClearRelease();
}

void Gimmick::JudgeAttack(float musicTime, int lane)
{
    notedata.JudgeAttack(musicTime, lane);
}

void Gimmick::SERelease()
{
    notedata.SERelease();
}

void Gimmick::ResetResult()
{
    notedata.ResetResult();
}

void Gimmick::UpdateScoreResult()
{
    notedata.UpdateScoreResult();
}

//コンボ数更新
void Gimmick::ComboUpdate(float x, float y, int num)
{
    if (num == 0)
    {
        ComboNum.SetUV(0.0f, 0.0f, 0.1f, 1.0f); // 0のUV
        ComboNum.SetPositon(Vector3(x, y, 0));
        ComboNum.Render();
        return;
    }

    int digits[10];
    int count = 0;

    while (num > 0)
    {
        digits[count++] = num % 10;
        num /= 10;
    }

    for (int i = 0; i < count; i++)
    {
        int n = digits[count - 1 - i];
        ComboNum.SetUV(0.1f * n, 0.0f, 0.1f, 1.0f);
        ComboNum.SetPositon(Vector3(x + 50 * i, y, 0));
        ComboNum.Render();
    }
}

void Gimmick::JudgeAuto(float musicTime)
{
    notedata.JudgeAuto(musicTime);
}

void Gimmick::SetAutoPlay(bool enable)
{
    notedata.SetAutoPlay(enable);
}

float Gimmick::GetSpeed()const
{
    return Config::notespeed;
}

// CSV読み込み時にdurationを読む
void Gimmick::LoadCSV(const std::string& filepath)
{
    notedata.SetNoteData(filepath);

    data.bpm = notedata.GetBPM();


}


//--------------------------------------
// 更新
//--------------------------------------
void Gimmick::Update(float musicTime)
{
    // ノーツ・小節線生成
    notedata.SpawnNote(musicTime);
    notedata.SpawnBarLines(musicTime);
    notedata.UpdateSpeed();
    notedata.UpdateSpawnZ();

    notedata.Update(musicTime);
}

bool Gimmick::IsAllNotesCleared()const
{
    return notedata.IsAllNotesCleared();
}

//--------------------------------------
// 描画
//--------------------------------------
void Gimmick::Draw()
{
    char buf[128];
    sprintf_s(buf,
        "notes=%d active=%d bars=%d\n",
        (int)notedata.notes.size(),
        (int)notedata.activeNotes.size(),
        (int)notedata.barLines.size());

    OutputDebugStringA(buf);

    notedata.Render();

    for (auto& n : notedata.activeNotes)
    {
        if (!n.mesh) continue;
        Shader::SetTransform(n.mesh->transform);
        n.mesh->Render();
    }


	RenderState::SetCullMode(D3D11_CULL_NONE); // 両面描画
    for (auto i = 0; i < notedata.verticalline.size(); i++) {
        decltype(auto) n = notedata.verticalline[i];
        if (!n) continue;
        n->scale = Vector3(0.1f, 1.0f, 1000.0f);
        n->position = Vector3(verticalLaneX[i], 0.1, -5.0f);
        n->Update();
        Shader::SetTransform(n->transform);
        n->Render();
    }
    RenderState::SetCullMode(D3D11_CULL_FRONT);

    RenderState::DepthEnable(false);

   
    for (auto& e : notedata.judgeEffectQueue)
    {
        if (e.type == 0)
        {
            Perfect.Render();
        }
        else if (e.type == 1)
        {
            Good.Render();
        }
        else if (e.type == 2)
        {
            Miss.Render();
        }
        else if (e.type == 10)
        {
            perfectEffect.SetPositon(e.position);
            perfectEffect.Render();
        }
        else if (e.type == 11)
        {
            goodEffect.SetPositon(e.position);
            goodEffect.Render();
        }
        else if (e.type == 12)
        {
            missEffect.SetPositon(e.position);
            missEffect.Render();
        }
    }

    if (ComboRender) {
        ComboUpdate(100, 100, notedata.GetComboCount());
        ComboUpdate(100, 200, notedata.GetPerfectCount());
        ComboUpdate(100, 300, notedata.GetGoodCount());
        ComboUpdate(100, 400, notedata.GetMissCount());
    }

    RenderState::DepthEnable(true);
}

void Gimmick::CreatePreviewNotes()
{
    notedata.CreatePreviewNotes();
}

void Gimmick::JudgePreview(float musicTime)
{
    notedata.JudgePreview(musicTime);
}

// 判定ラインの色を変更
void Gimmick::SetJudgeLineColor(float r, float g, float b, float alpha)
{
    judgeLineConfig.color = Vector3(r, g, b);
    judgeLineConfig.alpha = alpha;
}

#if _DEBUG
void Gimmick::Debuginfo()
{
    ImGui::Begin("Test_ComboNum");
    ImGui::Text("combo:%d", notedata.GetComboCount());
    ImGui::Text("perfectnum:%d", notedata.GetPerfectCount());
    ImGui::Text("goodnum:%d", notedata.GetGoodCount());
    ImGui::Text("missnum:%d", notedata.GetMissCount());
    ImGui::Text("ScrollSpeed");
    ImGui::Separator();
    ImGui::Text("AUTO PLAY : %s", autoPlay ? "ON" : "OFF");
    ImGui::End();
}
#endif