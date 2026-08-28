#pragma once

#include <vector>
#include "FbxMesh.h"
#include "Time.h"
#include "Math/Matrix.h"
#include "Gimmick.h"
#include "Render2D.h"
#include "Input/Input.h"
#include "Player.h"
#include "mgLib/mgShader.h"
#include "Camera/Camera.h"

#include "DxSystem.h"
#include "DXShader.h"

class Stage
{
public:
    Stage();
    ~Stage();

    bool Initialize();
    void Update(GameTime& time);
    void Draw();
    float GetScrollSpeed() { return scrollspeed; }

    void SetPlayer(Player* p);

//レーンカメラ変更変数
    int cameraY = 6;
 
#if _DEBUG
    void DebugInfomation();
#endif

private:
    FBXMesh* stageMesh;   // ステージ背景（固定）
    GameTime* gametime;   // 経過時間
    Gimmick* gimmick;     // ギミック管理クラス
    FBXMesh* judgeline;   // 判定ライン
    mgShader laneShader;  //レーンShader
    Camera camera;
  
    Player* player = nullptr;

    float angle;
    float scrollspeed;    //ステージスクロールスピード

    
    
};
