#include "SceneOption.h"
#include "Input/Input.h"
#include "config/config.h"
#include "Gimmick.h"
#include "imgui/imgui.h"
#include "CommonUI.h"
#include "GameSystem.h"


void SceneOption::Initialize(SceneState& scene)
{
    PreviewStage.Initialize();
    PreviewGimmick.Initialize();

    PreviewGimmick.CreatePreviewNotes();

    previewTime = 0.0f;
}

void SceneOption::Update(SceneState& scene,Gimmick& gimmick)
{
    if (Input::IsKeyTriggerd(VK_ESCAPE))
    {
        Config::Save();
        scene.Set(SELECTMODE);
        return;
    }

    gimmick.SetSpeed(Config::notespeed);

    //========================================
    // Preview
    //========================================

    previewTime += DeltaTime;

    PreviewGimmick.SetSpeed(Config::notespeed);
    PreviewGimmick.Update(previewTime);

    if (Input::IsKeyTriggerd(VK_SPACE))
    {
        PreviewGimmick.JudgePreview(previewTime);
    }

    // 30小節終了後、最初から
    if (previewTime >= 62.0f)
    {
        previewTime = 0.0f;

        PreviewGimmick.ClearRelease();
        PreviewGimmick.CreatePreviewNotes();
    }
}

void SceneOption::Render(SceneState& scene, UI& ui)
{
    ui.titlepix.Render();

    // 設定UI
    ImGui::SetNextWindowSize(
        ImVec2(1920, 1080),
        ImGuiCond_Always
    );

    ImGui::SetNextWindowPos(
        ImVec2(0, 0),
        ImGuiCond_Always
    );

    ImGui::Begin(
        u8"設定",
        NULL,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse
    );

    ImGui::SetWindowFontScale(1.8f);

    ImGui::Text(u8"設定");
    ImGui::Separator();

    ImGui::Spacing();

    //==================================================
    // Game / Preview タブ
    //==================================================

    if (ImGui::BeginTabBar("OptionTab"))
    {
        //==================================================
        // Game
        //==================================================
        if (ImGui::BeginTabItem(u8"Game"))
        {
            ImGui::Spacing();

            // キーパターン
            ImGui::Text(u8"キーパターン");

            const char* patternNames[] =
            {
                u8"キー配置１(S,D,J,K,L)",
                u8"キー配置２(S,D,F,J,K)"
            };

            ImGui::Combo(
                "##KeyPattern",
                &Config::currentPattern,
                patternNames,
                IM_ARRAYSIZE(patternNames)
            );

            ImGui::Spacing();

            // ノーツ速度
            ImGui::Text(u8"ノーツスピード");

            ImGui::SliderFloat(
                "##Speed",
                &Config::notespeed,
                10.0f,
                40.0f
            );

            ImGui::SameLine();

            ImGui::InputFloat(
                "##SpeedInput",
                &Config::notespeed,
                1.0f,
                5.0f
            );
            ImGui::Spacing();

            ImGui::Text(u8"レーン角度");

            ImGui::SliderInt(
                "##LaneAngle",
                &Config::LaneAngle,
                6,
                13,
                "%d"
                );

            ImGui::Spacing();

            // Offset
            ImGui::Text(u8"判定オフセット");

            ImGui::SliderFloat(
                "##Offset",
                &Config::Offset,
                -0.05f,
                0.05f
            );

            ImGui::SameLine();

            ImGui::InputFloat(
                "##OffsetInput",
                &Config::Offset,
                0.001f,
                0.01f
            );

            ImGui::Spacing();

            // BGM
            ImGui::Text(u8"楽曲の音量");

            ImGui::SliderFloat(
                "##BGM",
                &Config::bgmVolume,
                0.0f,
                1.0f
            );

            ImGui::Spacing();

            // SE
            ImGui::Text(u8"効果音の音量");

            ImGui::SliderFloat(
                "##SE",
                &Config::seVolume,
                0.0f,
                1.0f
            );

            ImGui::Spacing();

            // Fullscreen
            ImGui::Checkbox(
                u8"フルスクリーン",
                &Config::fullscreen
            );

            ImGui::EndTabItem();
        }

        //==================================================
        // Preview
        //==================================================
        if (ImGui::BeginTabItem(u8"Preview"))
        {
            ImGui::Spacing();

            ImGui::Text(u8"プレビュー");

            ImGui::Spacing();

            ImGui::Text(u8"SPACEキーでノーツ取れるよ！");

            ImGui::Spacing();


            //==================================================
            // Preview用カメラ
            //==================================================

            DxSystem::ViewMatrix.LookAt(
                Vector3(0, Config::LaneAngle, 3),
                Vector3(0, 1, 0)
            );

            DxSystem::ProjectionMatrix.PerspectiveFov(
                1.0f,
                800.0f / 450.0f,
                0.1f,
                1500.0f
            );


            //==================================================
            // Preview描画
            //==================================================

            DxSystem::BeginPreviewRender();

            PreviewStage.Draw();

            PreviewGimmick.SetComboRender(false);
            PreviewGimmick.Draw();

            DxSystem::EndPreviewRender();


            //==================================================
            // ImGuiに表示
            //==================================================

            ImGui::Image(
                (ImTextureID)DxSystem::PreviewShaderResourceView,
                ImVec2(800.0f, 450.0f)
            );


            ImGui::EndTabItem();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::EndTabBar();
    }
    //==================================================
    // 保存
    //==================================================

    if (ImGui::Button(u8"保存"))
    {
        Config::Save();

        ImGui::OpenPopup("SaveComplete");
    }

    if (ImGui::BeginPopupModal(
        "SaveComplete",
        NULL,
        ImGuiWindowFlags_AlwaysAutoResize
    ))
    {
        ImGui::Text(u8"設定を保存しました");

        ImGui::Spacing();

        if (ImGui::Button(u8"完了", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::SameLine();

    //==================================================
    // 戻る
    //==================================================

    if (ImGui::Button(u8"戻る"))
    {
        scene.Set(TITLE);
    }

    ImGui::End();
}