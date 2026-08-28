// --------------------------------------
// Player.cpp
// --------------------------------------

#include "Player.h"

#include <vector>

#include "DXShader.h"
#include "FbxMesh.h"
#include "Time.h"
#include "config/config.h"
#include "Input/Input.h"
#include "imgui/imgui.h"
#include <string>

using namespace DirectX;

// --------------------------------------
// レーン座標（5K固定）
// --------------------------------------
static const float LaneX[5] =
{
    -2.0f,
    -1.0f,
     0.0f,
     1.0f,
     2.0f
};

// --------------------------------------
// コンストラクタ / デストラクタ
// --------------------------------------
Player::Player() :
    Position(0.0f, 0.0f, 0.0f),
    currentLane(2)
{
}

Player::~Player()
{
}

// --------------------------------------
// 初期化
// --------------------------------------
void Player::Initialize()
{
    currentLane = 2;

    Position =
    {
        LaneX[currentLane],
        0.0f,
        -0.65f
    };

    for (int i = 0; i < 5; i++)
    {
        prevKey[i] = false;
    }

    attackLanes.clear();
    attackReadIndex = 0;

    for (int i = 0; i < 5; i++) {
        laneParam.GlowColor[i] = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    }
}

// --------------------------------------
// 更新
// --------------------------------------
void Player::Update(float time)
{
    float dt = time;

    ResetAttackBuffer();

    UpdateInput();

    UpdateTransform();
}

// --------------------------------------
// 入力更新
// --------------------------------------
void Player::UpdateInput()
{
    for (int lane = 0; lane < 5; ++lane)
    {
        laneParam.GlowColor[lane] = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

        if (Input::IsKeyDown(Config::GetCurrentLaneKeys()[lane]))
        {
            laneParam.GlowColor[lane] = XMFLOAT4(0.1, 0.1, 0.9, 0.0);
        }

        // 判定用
        if (Input::IsKeyTriggerd(
            Config::GetCurrentLaneKeys()[lane]))
        {
            attackLanes.push_back(lane);
        }
    }
}
// --------------------------------------
// 攻撃バッファ初期化
// --------------------------------------
void Player::ResetAttackBuffer()
{
    attackLanes.clear();
    attackReadIndex = 0;
}

// --------------------------------------
// Transform更新
// --------------------------------------
void Player::UpdateTransform()
{
    Position.x = 0.0f;
}

// --------------------------------------
// 攻撃取得
// --------------------------------------
bool Player::ConsumeAttack(int& outLane)
{
    if (attackReadIndex >= attackLanes.size())
        return false;

    outLane = attackLanes[attackReadIndex];

    attackReadIndex++;

    return true;
}

// --------------------------------------
// 描画
// --------------------------------------
void Player::Draw()
{
}

// --------------------------------------
// ダメージ
// --------------------------------------
void Player::Damage(int damage)
{
}

// --------------------------------------
// 無敵開始
// --------------------------------------
void Player::StartInvincible(float time)
{
}

// --------------------------------------
// 無敵判定
// --------------------------------------
bool Player::IsInvincible() const
{
    return false;
}

const Player::LaneParam& Player::GetLaneParam() const
{
    return laneParam;
}

// --------------------------------------
// Debug
// --------------------------------------
#ifdef _DEBUG

void Player::DebugInfomation()
{
    ImGui::Begin("Player_Debug");

    ImGui::Text(
        "CurrentLane : %d",
        currentLane
    );

    ImGui::Text(
        "AttackCount : %d",
        (int)attackLanes.size()
    );

    ImGui::End();
}

#endif