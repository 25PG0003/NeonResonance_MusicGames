#pragma once

#include <vector>

#include "FbxMesh.h"
#include <DirectXMath.h>

using namespace DirectX;


class Player
{
private:

    // --------------------------------------
    // 内部更新
    // --------------------------------------
    void UpdateInput();

    void UpdateTransform();

    void ResetAttackBuffer();


private:

    bool prevKey[5] = {};
    float attackBuffer[5] = {};

    // --------------------------------------
    // 座標
    // --------------------------------------
    Vector3 Position;

    // --------------------------------------
    // レーン
    // --------------------------------------
    int currentLane = 0;

    // --------------------------------------
    // 攻撃情報
    // --------------------------------------
    std::vector<int> attackLanes;

    int attackReadIndex = 0;

    struct LaneParam
    {
        XMFLOAT4 GlowColor[5];
    };

    LaneParam laneParam;



public:

    // --------------------------------------
    // コンストラクタ / デストラクタ
    // --------------------------------------
    Player();
    ~Player();

    // --------------------------------------
    // 基本処理
    // --------------------------------------
    void Initialize();
    void Update(float time);
    void Draw();

    // --------------------------------------
    // ダメージ / 無敵
    // --------------------------------------
    void Damage(int damage);

    void StartInvincible(float time);

    bool IsInvincible() const;

    // --------------------------------------
    // 攻撃取得（同時押し対応）
    // --------------------------------------
    bool ConsumeAttack(int& outLane);

    //Shader関連
    const LaneParam& GetLaneParam() const;

#if _DEBUG
    void DebugInfomation();
#endif

    // --------------------------------------
    // Getter
    // --------------------------------------
    const Vector3& GetPosition() const
    {
        return Position;
    }

    Vector3 GetForward() const
    {
        return Vector3(0.0f, 0.0f, 1.0f);
    }

    // --------------------------------------
    // 判定ライン
    // --------------------------------------
    static constexpr float HitLineZ = 0.0f;

};