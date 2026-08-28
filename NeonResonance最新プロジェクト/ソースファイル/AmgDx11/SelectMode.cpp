#include "SelectMode.h"

#include "CommonUI.h"
#include "RenderState.h"
#include "Input/Input.h"
#include "Gimmick.h"
#include <windows.h>
#include "Scenemaingame.h"
#include <shobjidl.h>
#include <string>
#include <filesystem>
#include "GameSystem.h"
#include <cmath>


#include "imgui/imgui.h"



#include "Scene/SceneState.h"

bool SelectMode::Initialize(SceneState& scene)
{
    ui.Initialize();
    //難易度選択画面画像読み込み
    difficultScene.Initialize("Assets/difficult/selectmusic.png");
    difficultScene.SetSize(1920, 1080);
    difficultScene.SetPositon(Vector3(0, 0, 0));

    optionHint[0].Initialize("Assets/SelectMode/SelectModetoOption.png");
    optionHint[1].Initialize("Assets/SelectMode/SelectOption.png");
    
    SceneFreePlay[0].Initialize("Assets/SelectMode/freeplay.png");
    SceneFreePlay[1].Initialize("Assets/SelectMode/Selectfreeplay.png");

    SceneChartLoad[0].Initialize("Assets/SelectMode/chartload.png");
    SceneChartLoad[1].Initialize("Assets/SelectMode/SelectChartLoad.png");

    SceneEditor[0].Initialize("Assets/SelectMode/editor.png");
    SceneEditor[1].Initialize("Assets/SelectMode/SelectEditor.png");

    SceneTitle[0].Initialize("Assets/SelectMode/return_title.png");
    SceneTitle[1].Initialize("Assets/SelectMode/SelectTitle.png");

    for (int i = 0; i < 2; i++)
    {
        optionHint[i].SetPositon(Vector3(WIDTH / 2 + 50, HEIGHT / 2 + 400, 0));
        SceneFreePlay[i].SetPositon(Vector3(550, 400, 0));
        SceneChartLoad[i].SetPositon(Vector3(550, 520, 0));
        SceneEditor[i].SetPositon(Vector3(550, 640, 0));
        SceneTitle[i].SetPositon(Vector3(10, 950, 0));
    }

    for (int i = 4; i < 4; i++)
    {
        menuAnim[i] = 0.0f;
    }
    
    
    return true;
}

void SelectMode::Release()
{
    //何
}

float SelectMode::EaseOutCubic(float t)
{
    return 1.0f - powf(1.0f - t, 3.0f);
}

std::string SelectMode::OpenChartFile()
{
    IFileOpenDialog* pFileOpen = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_ALL,
        IID_PPV_ARGS(&pFileOpen));

    if (FAILED(hr))
        return "";

    COMDLG_FILTERSPEC filters[] =
    {
        { L"CSV Files", L"*.csv" },
        { L"All Files", L"*.*" }
    };

    pFileOpen->SetFileTypes(ARRAYSIZE(filters), filters);

    // Assets/chart を初期フォルダにする
    IShellItem* folder = nullptr;

    std::filesystem::path folderPath =
        std::filesystem::current_path() / "Assets" / "createChart";

    std::wstring folderW = folderPath.wstring();

    hr = SHCreateItemFromParsingName(
        folderW.c_str(),
        nullptr,
        IID_PPV_ARGS(&folder)
    );

    if (SUCCEEDED(hr))
    {
        pFileOpen->SetDefaultFolder(folder);
        pFileOpen->SetFolder(folder);
        folder->Release();
    }


    DWORD options = 0;
    pFileOpen->GetOptions(&options);
    pFileOpen->SetOptions(options | FOS_NOCHANGEDIR);
    hr = pFileOpen->Show(nullptr);

    if (FAILED(hr))
    {
        pFileOpen->Release();
        return "";
    }

    IShellItem* pItem = nullptr;
    hr = pFileOpen->GetResult(&pItem);

    if (FAILED(hr))
    {
        pFileOpen->Release();
        return "";
    }

    PWSTR filePath = nullptr;
    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);

    if (FAILED(hr))
    {
        pItem->Release();
        pFileOpen->Release();
        return "";
    }

    char path[MAX_PATH]{};

    WideCharToMultiByte(
        CP_ACP,
        0,
        filePath,
        -1,
        path,
        MAX_PATH,
        nullptr,
        nullptr
    );


    std::string result = path;

    CoTaskMemFree(filePath);
    pItem->Release();
    pFileOpen->Release();

    return result;
}

const std::string& SelectMode::GetSelectedChartPath() const
{
    return selectedChartPath;
}

void SelectMode::Update(SceneState& scene, AudioSource* audio, Gimmick& gimmick, Scenemaingame& maingame)
{
    music = audio;

    //マウスの現在位置を取得
    POINT mouse;
    GetCursorPos(&mouse);
    ScreenToClient(GetForegroundWindow(), &mouse);

    //メニュー当たり判定
    int mouseX = mouse.x;
    int mouseY = mouse.y;

    selectedMenu = -1;
    if (mouseX >= 10 && mouseX <= 550 + 550)
    {
        if (mouseY >= 400 && mouseY <= 500)
        {
            selectedMenu = 0;
        }
        else if (mouseY >= 520 && mouseY <= 620)
        {
            selectedMenu = 1;
        }
        else if (mouseY >= 640 && mouseY <= 740)
        {
            selectedMenu = 2;
        }
        else if (mouseY >= 950 && mouseY <= 1050)
        {
            selectedMenu = 3;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if (selectedMenu == i)
        {
            menuAnim[i] += DeltaTime / 0.2f;
        }
        else
        {
            menuAnim[i] -= DeltaTime / 0.2f;
        }

        menuAnim[i] = std::clamp(menuAnim[i], 0.0f, 1.0f);
    }

    if (selectedMenu >= 0 && Input::IsMouseTriggerd(0))
    {
        switch (selectedMenu)
        {
        case 0:
            // FreePlay
            scene.SetNextScene(MainGame);
            scene.Set(SELECTMUSIC);
            break;

        case 1:
            // ChartLoad
            loadchart = true;
            break;

        case 2:
            // Editor
            scene.SetNextScene(EditorMode);
            scene.Set(SELECTMUSIC);
            break;

        case 3:
            // Title
            scene.Set(TITLE);
            break;
        }
    }


    if (music == nullptr) return;

    if (Input::IsKeyTriggerd(VK_RETURN))
    {
        scene.SetNextScene(MainGame);
        scene.Set(SELECTMUSIC);
    }

    if (Input::IsKeyTriggerd('T'))
    {
        scene.SetNextScene(MainGame);
        scene.Set(SELECTMUSIC);
    }

    if (Input::IsKeyTriggerd('K'))
    {
        loadchart = true;
    }

    if (Input::IsKeyTriggerd(VK_LSHIFT))
    {
        showChartSelect = !showChartSelect;
    }

    if (Input::IsKeyTriggerd(VK_F10))
    {
        scene.Set(Option);
    }

    if (Input::IsKeyTriggerd('E'))
    {
        scene.SetNextScene(EditorMode);
        scene.Set(SELECTMUSIC);
    }

    if (Input::IsKeyTriggerd(VK_ESCAPE))
    {
        scene.Set(TITLE);
    }

    if (showChartSelect)
    {
        ImGui::Begin("My Charts");
        ImGui::SetWindowFontScale(3.0f);

        std::filesystem::path dir =
            std::filesystem::current_path() / "Assets" / "createChart";

        static int selected = -1;

        int i = 0;
        for (auto& file : std::filesystem::directory_iterator(dir))
        {
            if (file.path().extension() != ".csv") continue;

            std::string name = file.path().stem().string();

            bool isSelected = (selected == i);

            if (ImGui::Selectable(name.c_str(), isSelected))
            {
                selected = i;
            }

            // 選択してるパス保持
            if (isSelected)
            {
                selectedChartPath = file.path().string();
            }

            i++;
        }

        if (selected >= 0)
        {
            if (ImGui::Button("START"))
            {
                gimmick.LoadCSV(selectedChartPath);
                maingame.LoadMusicFromChart(gimmick, *music);

                scene.Set(MainGame);

                showChartSelect = false;
                selected = -1;
            }
        }

        ImGui::End();
    }
}

void SelectMode::Render(SceneState& scene)
{
    RenderState::DepthEnable(false);

    ui.titlepix.Render();

    difficultScene.Render();

    // FreePlay
    {
        float t = EaseOutCubic(menuAnim[0]);

        float x = 550.0f + 20.0f * t;

        SceneFreePlay[selectedMenu == 0 ? 1 : 0]
            .SetPositon(Vector3(x, 400, 0));

        SceneFreePlay[selectedMenu == 0 ? 1 : 0].Render();
    }

    // ChartLoad
    {
        float t = EaseOutCubic(menuAnim[1]);

        float x = 550.0f + 20.0f * t;

        SceneChartLoad[selectedMenu == 1 ? 1 : 0]
            .SetPositon(Vector3(x, 520, 0));

        SceneChartLoad[selectedMenu == 1 ? 1 : 0].Render();
    }

    // Editor
    {
        float t = EaseOutCubic(menuAnim[2]);

        float x = 550.0f + 20.0f * t;

        SceneEditor[selectedMenu == 2 ? 1 : 0]
            .SetPositon(Vector3(x, 640, 0));

        SceneEditor[selectedMenu == 2 ? 1 : 0].Render();
    }

    // Title
    {
        float t = EaseOutCubic(menuAnim[3]);

        float x = 10.0f + 20.0f * t;

        SceneTitle[selectedMenu == 3 ? 1 : 0]
            .SetPositon(Vector3(x, 950, 0));

        SceneTitle[selectedMenu == 3 ? 1 : 0].Render();
    }
}