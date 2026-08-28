#include "Title.h"
#include "Scene/SceneState.h"
#include "GameSystem.h"
#include "Input/Input.h"
#include "RenderState.h"
#include "CommonUI.h"

#include "imgui/imgui.h"


bool Title::Initialize(SceneState& scene)
{
    ui.Initialize();
    //初期シーン情報
    scene.Set(SCENE_GAME::TITLE);

	//シーン遷移用のボタン画像読み込み
	SceneButton.Initialize("Assets/title/pressenter.png");
	SceneButton.SetSize(400, 400);

	//タイトル文字読み込み
	titleSprite.Initialize("Assets/title/titlesprite.png");
	
    
	//タイトルアニメーション用変数初期化
	titleAnimTime = 0.0f;

	return true;
}

void Title::Release()
{
	//何もしない
}

void Title::Update(SceneState& scene)
{
    //State管理
    if (scene.Get() == TITLE) //現在のシーンがタイトル画面の時
    {
        titleAnimTime += DeltaTime;  //アニメーションフレーム更新

        if (Input::IsKeyTriggerd(VK_RETURN))
        {
            scene.Set(SELECTMODE); //難易度選択画面へ遷移
        }

        if (Input::IsKeyTriggerd('O'))
        {
            scene.Set(Option); //オプション画面へ遷移
        }

        if (Input::IsKeyTriggerd(VK_ESCAPE))
        {
            ImGui::SetNextWindowSize(ImVec2(710, 240), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(WIDTH / 2 - 380, HEIGHT / 2 - 100), ImGuiCond_Always);
            ImGui::OpenPopup("ゲームを終了しますか?");

        }
        if (ImGui::BeginPopupModal("ゲームを終了しますか?", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::SetWindowFontScale(5.0f);


            {
                ImGui::Dummy(ImVec2(0, 20));
                ImGui::SameLine();

                if (ImGui::Button("はい:y") || Input::IsKeyTriggerd('Y'))
                {
                    PostQuitMessage(0);
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();

                ImGui::Dummy(ImVec2(50, 0));

                ImGui::SameLine();

                if (ImGui::Button("いいえ:n") || Input::IsKeyTriggerd('N'))
                {
                    scene.Set(TITLE);
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }
        return;
    }
}

void Title::UIAnim()
{
    //sinfでタイトル画面上下に動かす
    float offsetY = tanf(titleAnimTime * 1.0f) * 25.0f;
    float buttonoffsetY = tanf(titleAnimTime * 1.0f) * 50.0f;

    float offsetX = sinf(titleAnimTime * 1.2f) * 100.0f;
    
}

void Title::Render(SceneState& scene)
{
    //タイトル画面の時
    if (scene.Get() == TITLE)
    {
      
        float offsetY = sinf(titleAnimTime * 1.0f) * 25.0f;
        //タイトル画像の座標設定
        titleSprite.SetPositon(Vector3(-80, -240 + offsetY, 0));
        SceneButton.SetPositon(Vector3(720, 620 + offsetY, 0));
        //Depth無効
        RenderState::DepthEnable(false);

        //タイトル描画
        ui.titlepix.Render();
        titleSprite.Render();
        SceneButton.Render();
        return;
    }
}