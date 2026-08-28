#include "SceneEditor.h"
#include "Scene/SceneState.h"
#include "imgui/imgui.h"
#include "GameSystem.h"
#include "Input/Input.h"

void SceneEditor::Initialize(SceneState& scene,Editor& editor)
{
    editor.Initialize();
}

void SceneEditor::EditorUpdate(SceneState& scene, GameTime& gameTime,Editor& editor)
{

    //エディター画面の時
    if (scene.Get() == EditorMode)
    {
       
        editor.Update();
     

        if (Input::IsKeyTriggerd(VK_ESCAPE))
        {
            if (editor.IsSaved())
            {
                editor.ResetEditor();
                gameTime.Reset();
                scene.Set(SELECTMUSIC);
        
            }
            scene.Set(ConfirmExit);
            return;
        }

    }



    if (scene.Get() == SCENE_GAME::ConfirmExit)
    {

        if (scene.Get() == ConfirmExit && !showSaveSuccessPopup)
        {
            ImGui::OpenPopup("ExitConfirm");

            if (ImGui::BeginPopupModal("ExitConfirm", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Chart Save?");

                if (ImGui::Button("yes : y") || Input::IsKeyTriggerd('Y'))
                {
                    editor.SaveChart();

                    
                    gameTime.Reset();

                    showSaveSuccessPopup = true;

                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                    return;
                }

                ImGui::SameLine();

                if (ImGui::Button("no : n") || Input::IsKeyTriggerd('N'))
                {
                    scene.Set(TITLE);
                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                    return;
                }

                ImGui::SameLine();

                if (ImGui::Button("cancel : c") || Input::IsKeyTriggerd('C'))
                {
                    scene.Set(EditorMode);
                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                    return;
                }

                ImGui::EndPopup();
            }
        }

        if (showSaveSuccessPopup)
        {
            ImGui::SetNextWindowPos(ImVec2(WIDTH / 2, HEIGHT / 2), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::OpenPopup("SaveChart");
        }

        if (ImGui::BeginPopupModal("SaveChart", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
			
            ImGui::Text("Save Success");

            if (ImGui::Button("OK"))
            {
                showSaveSuccessPopup = false;
                editor.ResetEditor();
                scene.Set(TITLE);
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
}

void SceneEditor::Render(SceneState& scene,Editor& editor,Editor::EditorData& data)
{
    editor.Render(data);
    editor.DebugGUI();
}


void SceneEditor::UnderConstructionUpdate(SceneState& scene,Editor& editor)
{
    if (scene.Get() == UnderConstruction)
    {
        ImGui::OpenPopup("Notice");

        if (ImGui::BeginPopupModal(
            "Notice",
            NULL,
            ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Editor is under construction.");
            ImGui::Text("Return to title.");

            if (ImGui::Button("OK") || Input::IsKeyTriggerd(VK_RETURN))
            {
                scene.Set(TITLE);
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
}