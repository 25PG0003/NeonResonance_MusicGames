// sceneTest.cpp
#include "SceneTest.h"
#include "Texture.h"
#include "RenderState.h"
#include "GameSystem.h"
#include "Player.h"
#include "Input/Input.h"
#include "Stage.h"
#include"editor/Editor.h"
#include "Scene/SceneState.h"
#include "config/config.h"
#include "CommonUI.h"

#include <windows.h>
#include <shobjidl.h>
#include <string>


#include "imgui/imgui.h"


SceneTest::SceneTest()
{
    
}

// デストラクタ
SceneTest::~SceneTest()
{
   
}

bool SceneTest::Initialize()
{
    ui.Initialize();

    gimmick.Initialize();
    stage.Initialize();
    stage.SetPlayer(&pl);
    shader.Activate();
    pl.Initialize();
    selectmusic.Initialize(scene);
    sceneeditor.Initialize(scene, editor);
    option.Initialize(scene);
    maingame.Initialize(scene,music,notedata);
    result.Initialize(scene);

    //タイトル画像読み込みなど
    title.Initialize(scene);
    selectmode.Initialize(scene);

    OptionToTitleHint.Initialize("Assets/option/TitleHint.png");
    OptionToTitleHint.SetSize(200, 120);
    OptionToTitleHint.SetPositon(Vector3(500, 200, 0));
    
    KeyConfigHint.Initialize("Assets/maingame/keyhint.png");
    KeyConfigHint.SetSize(100, 100);


    // 基本シェーダ
    shader.Create(L"Assets/shader/3D.fx", "VSMain", "PSMain");

    DxSystem::DeviceContext->VSSetConstantBuffers(0, 1, &Shader::CBBaseMatrix);
    DxSystem::DeviceContext->PSSetConstantBuffers(0, 1, &Shader::CBBaseMatrix);

    //メンバ変数初期化
    editor.start = false;
    return true;
}



void SceneTest::SceneTestRelease()
{
    

    //ノーツ解放
    gimmick.Release();

    title.Release();

    selectmusic.Release(scene);
}

void SceneTest::Update()
{
    if (scene.Get() == TITLE) {
        title.Update(scene);
    }

    if (scene.Get() == SELECTMODE)
    {
        selectmode.Update(scene, &music, gimmick,maingame);
    }
    
    if (scene.Get() == SELECTMUSIC)
    {
        if (selectmusic.IsLoadChart())
        {
           maingame.ResetGame(editor, gameTime, gimmick, music);

            gimmick.LoadCSV(selectmusic.GetSelectedChart());

            maingame.LoadMusicFromChart(gimmick, music);

            selectmusic.ResetLoadChart();
        }

        selectmusic.Update(scene,gimmick,music,maingame,editor,data);
        return;
    }

    

    if (scene.Get() == Difficult) //現在のシーンが難易度選択画面の時
    {
        if (Input::IsKeyTriggerd(VK_RETURN))
        {
            scene.Set(MainGame); //Enterキーでメインゲームへ遷移
        }
        if (Input::IsKeyTriggerd(VK_ESCAPE))
        {
            scene.Set(SELECTMODE); //ESCキーでセレクト画面へ戻る
        }
        
        return;
    }

    //オプション画面の時
    if (scene.Get() == Option)
    {
        option.Update(scene, gimmick);
        return;
    }

    if (scene.Get() == EditorMode || scene.Get() == ConfirmExit)
    {
        sceneeditor.EditorUpdate(scene, gameTime,editor);
        return;
    }
   

    if (scene.Get() == MainGame) // MainGameの場合
    {
        maingame.Update(scene, gimmick, editor, gameTime, pl, stage,music);
        return;
    }
    
    if (scene.Get() == Result)
    {
        RenderState::DepthEnable(false);
        result.Update(scene, maingame, editor, gameTime, gimmick,music);
        return;
    }

    if (ImGui::BeginPopupModal(
        "Chart Load Failed",
        NULL,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Failed to load chart.");

        if (ImGui::Button("OK"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void SceneTest::Render()
{
#if _DEBUG
     ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(
        (float)DxSystem::ScreenWidth,
        (float)DxSystem::ScreenHeight
    );
#endif
    //画面クリア
    DxSystem::Clear(0/255.0f,0/255.0f,0/255.0f);
    
    if (scene.Get() == TITLE) {
        title.Render(scene);
        return;
    }

    if (scene.Get() == SELECTMODE)
    {
        selectmode.Render(scene);
        return;
    }

    if (scene.Get() == SELECTMUSIC)
    {
        RenderState::DepthEnable(false);
        selectmusic.Render(scene);
        return;
    }

    if (scene.Get() == Option)
    {
        RenderState::DepthEnable(false);
        option.Render(scene,ui);
        return;
    }
    
    if (scene.Get() == EditorMode)
    {
        gimmick.SetComboRender(false);
        sceneeditor.Render(scene, editor, data); 
        return;
    }

    //Depth有効
    RenderState::DepthEnable(true);

    // =========================
    // 通常描画
    // =========================
    shader.Activate(); //通常Shader有効

    //UI関連
    //難易度選択画面だったら？
    if (scene.Get() == Difficult)
    {
        //Depth無効にして描画
        RenderState::DepthEnable(false);
       
    }

    //メインゲームの場合は？
    if (scene.Get() == MainGame)
    {
        shader.Activate();
        maingame.Render(scene, stage, gimmick, editor);
    }

    if (scene.Get() == Result)
    {
        result.Render(scene, gimmick);
    }

   

#if _DEBUG
    DebugGUI();
#endif
}


#if _DEBUG
void SceneTest::DebugGUI()
{

    
    pl.DebugInfomation();
    gimmick.Debuginfo();
    stage.DebugInfomation();



    ImGui::Begin("operate");
    ImGui::Text("J:note -> y++");
    ImGui::Text("Space: Time Start");
    ImGui::Text("A,D: move");
    ImGui::Text("DisplaySize: %.1f %.1f",
        ImGui::GetIO().DisplaySize.x,
        ImGui::GetIO().DisplaySize.y);
    ImGui::Text("now = %f", editor.now);
    ImGui::End();
}
#endif