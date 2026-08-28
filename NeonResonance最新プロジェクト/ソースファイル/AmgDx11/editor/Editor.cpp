#include "../time.h"
#include "../AudioSystem/AudioVoice.h"
#include "Editor.h"
#include "../Input/Input.h"
#include "../GameSystem.h"
#include "../Scene/SceneState.h"
#include <filesystem>

#if USE_IMGUI
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#endif

#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>


// Windows関連のh
#define NOMINMAX
#include <windows.h>
#include <shobjidl.h>
#include <propsys.h>
#include <propkey.h>
#include <propvarutil.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "propsys.lib")

#include "../Render2D.h"

#include"../musicdata.h"



static const std::filesystem::path ProjectRoot =
std::filesystem::current_path();

void Editor::PushUndo()
{
    undoStack.push_back({ notes });
    redoStack.clear();
}
void Editor::Undo()
{
    if (undoStack.empty()) return;
    redoStack.push_back({ notes });
    notes = undoStack.back().notes;
    undoStack.pop_back();
}
void Editor::Redo()
{
    if (redoStack.empty()) return;
    undoStack.push_back({ notes });
    notes = redoStack.back().notes;
    redoStack.pop_back();
}


// =======================
// 曲時間取得関数
// =======================
static float GetAudioLengthSec(const char* path)
{
    CoInitialize(nullptr);

    wchar_t wpath[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, path, -1, wpath, MAX_PATH);

    IPropertyStore* store = nullptr;
    float len = 0.0f;

    if (SUCCEEDED(SHGetPropertyStoreFromParsingName(
        wpath, nullptr, GPS_DEFAULT, IID_PPV_ARGS(&store))))
    {
        PROPVARIANT var;
        PropVariantInit(&var);

        if (SUCCEEDED(store->GetValue(PKEY_Media_Duration, &var)))
            len = float(var.uhVal.QuadPart / 10000000.0);

        PropVariantClear(&var);
        store->Release();
    }

    CoUninitialize();
    return len;
}

void Editor::LoadFromGimmick(const EditorData& data)
{
    musicPath = data.musicPath;
    bpm = data.bpm;

    music.Stop();
    music.Release();

    if (!music.Load(musicPath.c_str(), false))
    {
        OutputDebugStringA("LoadFromGimmick music.Load failed\n");
        return;
    }

    songLength = GetAudioLengthSec(musicPath.c_str());

    if (songLength <= 0.0f)
        songLength = 300.0f;
}

// =======================
// CSV読み込み
// =======================
void Editor::SaveCSV(const char* path)
{
    if (musicPath.empty())
    {
        MessageBoxA(nullptr, "Music path is empty.", "Save CSV Failed", MB_OK);
        return;
    }

    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        MessageBoxA(nullptr, path, "CSV Open Failed", MB_OK);
        return;
    }

    ofs << "#BPM," << bpm << "\n";
    ofs << "#OFFSET," << offset << "\n";
    ofs << "#MUSIC," << musicPath << "\n";

    for (const auto& n : notes)
    {
        ofs << n.time << "," << n.lane << ",0," << n.duration << "\n";
    }

    isSaved = true;
}

void Editor::LoadCSV(const char* path)
{
    notes.clear();

    std::ifstream ifs(path);
    if (!ifs.is_open()) return;

    std::string line;
    while (std::getline(ifs, line))
    {
        // UTF-8 BOM除去
        if (line.size() >= 3 &&
            (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB &&
            (unsigned char)line[2] == 0xBF)
        {
            line.erase(0, 3);
        }

        if (line.empty())
            continue;

        // ヘッダ解析
        if (line[0] == '#')
        {
            std::stringstream ss(line.substr(1));

            std::string key;
            std::string value;

            std::getline(ss, key, ',');
            std::getline(ss, value, ',');

            if (key == "BPM")
            {
                bpm = std::stof(value);
            }
            else if (key == "OFFSET")
            {
                offset = std::stof(value);
            }
            else if (key == "MUSIC")
            {
                musicPath = value;

                music.Stop();
                music.Release();

                bool result = music.Load(musicPath.c_str(), false);

                if (!result)
                {
                    MessageBoxA(
                        nullptr,
                        musicPath.c_str(),
                        "Music Load Failed",
                        MB_OK
                    );
                }
                else
                {
                    songLength = GetAudioLengthSec(musicPath.c_str());
                }
            }

            

            if (songLength <= 0.0f)
            {
                songLength = 300.0f;
            }

            isSaved = true;
            continue;
        }

        // ノーツ読み込み
        std::stringstream ss(line);
        std::string item;
        ChartNote n{};
        int dummy = 0;

        std::getline(ss, item, ',');
        n.time = std::stof(item);

        std::getline(ss, item, ',');
        n.lane = std::stoi(item);

        std::getline(ss, item, ',');
        dummy = std::stoi(item);

        if (n.lane < 0 || n.lane >= 5)
            continue;

        std::getline(ss, item, ',');
        n.duration = std::stof(item);

        n.sePlayed = false;

        notes.push_back(n);
    }

    std::sort(notes.begin(), notes.end(),
        [](auto& a, auto& b)
        {
            return a.time < b.time;
        });

    ResetSEFlags();

    rawPlayTime = 0.0f;
    viewTime = 0.0f;
    editStartTime = 0.0f;
    startTime = 0.0f;
    playing = false;
}
// =======================
// ファイルダイアログ
// =======================
static bool OpenFile(char* out,DWORD size,const char* filter,const char* initialDir = nullptr)
{
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = out;
    ofn.nMaxFile = size;
    ofn.lpstrInitialDir = initialDir;

    ofn.Flags =
        OFN_FILEMUSTEXIST |
        OFN_PATHMUSTEXIST |
        OFN_NOCHANGEDIR |
        OFN_DONTADDTORECENT;

    return GetOpenFileNameA(&ofn);
}
static bool SaveFile(char* out,DWORD size,const char* filter,const char* initialDir = nullptr)
{
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);

    ofn.lpstrFilter = filter;
    ofn.lpstrFile = out;
    ofn.nMaxFile = size;

    // 追加
    ofn.lpstrInitialDir = initialDir;

    ofn.lpstrDefExt = "csv";

    ofn.Flags =
        OFN_PATHMUSTEXIST |
        OFN_OVERWRITEPROMPT |
        OFN_NOCHANGEDIR;

    return GetSaveFileNameA(&ofn);
}
static bool OpenMusicFile(char* out, DWORD size)
{
    OPENFILENAMEA ofn{};
    ZeroMemory(&ofn, sizeof(ofn));
    ZeroMemory(out, size);

    char initialDir[MAX_PATH]{};

    std::filesystem::path musicDir =
        ProjectRoot / "Assets" / "music";

    strcpy_s(initialDir, musicDir.string().c_str());

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter =
        "Audio Files\0*.wav;*.mp3\0"
        "All Files\0*.*\0";
    ofn.lpstrFile = out;
    ofn.nMaxFile = size;

    ofn.lpstrInitialDir = initialDir;

    ofn.Flags =
        OFN_FILEMUSTEXIST |
        OFN_PATHMUSTEXIST |
        OFN_NOCHANGEDIR;

    return GetOpenFileNameA(&ofn);
}
void Editor::SaveChart()
{
    char path[MAX_PATH] = {};
    char chartDir[MAX_PATH] = {};

    std::filesystem::path dir =
        ProjectRoot / "Assets" / "createChart";

    strcpy_s(chartDir, dir.string().c_str());

    std::filesystem::path defaultPath =
        dir / "new_chart.csv";

    strcpy_s(path, defaultPath.string().c_str());

    if (SaveFile(
        path,
        MAX_PATH,
        "CSV\0*.csv\0",
        chartDir))
    {
        SaveCSV(path);
    }
}
bool Editor::Initialize()
{
    //２D画像読み込み
    note.Initialize("Assets/editor/note.png");
    for (int i = 0; i < 5; i++)
    {
        lane[i].Initialize("Assets/editor/lane.png");
    }
    barlines.Initialize("Assets/editor/barline.png");
    playhead.Initialize("Assets/editor/judgeline.png");

    laneLine.Initialize("Assets/editor/lineLine.png");


    noteSE.Load("SE1","Assets/SE/perfect.wav", false);

    return true;
}

void Editor::LoadMusic(const std::string& path)
{
    musicPath = path;
    OutputDebugStringA(("LoadMusic path = " + musicPath + "\n").c_str());

    music.Stop();
    music.Release();

    if (!music.Load(musicPath.c_str(), false))
    {
        OutputDebugStringA("LoadMusic failed\n");
        return;
    }

    songLength = GetAudioLengthSec(musicPath.c_str());
    
    if (songLength <= 0.0f)
        songLength = 300.0f;
}

void Editor::Update()
{
    // ================= ショートカット =================

  // Space : Play / Stop
    if (Input::IsKeyTriggerd(VK_SPACE))
    {
        if (playing)
        {
            music.Stop();
            ResetSEFlags();

            viewTime = rawPlayTime;
            playing = false;
        }
        else
        {
            music.Stop();
            music.Seek(viewTime);
            music.Play();

            ResetSEFlags();
            playing = true;
        }
    }

    // R : Reset
    if (Input::IsKeyTriggerd('R'))
    {
        music.Stop();
        music.Seek(0.0f);

        ResetSEFlags();

        rawPlayTime = 0.0f;
        viewTime = 0.0f;
        editStartTime = 0.0f;
        startTime = 0.0f;

        playing = false;
    }

    // Ctrl + Z : Undo
    if ((GetKeyState(VK_CONTROL) & 0x8000) && Input::IsKeyTriggerd('Z'))
    {
        Undo();
    }

    // Ctrl + Y : Redo
    if ((GetKeyState(VK_CONTROL) & 0x8000) && Input::IsKeyTriggerd('Y'))
    {
        Redo();
    }

    //再生時間更新
    if (playing)
    {
        rawPlayTime = music.GetPlayTime();
        if (rawPlayTime < 0.0f) rawPlayTime = 0.0f;
        viewTime = rawPlayTime;
    }

    POINT p;
    GetCursorPos(&p);
    ScreenToClient(GetActiveWindow(), &p);

    float areaX = editorX;
    float areaY = editorY;
    float areaW = laneWidth * laneCount;
    float areaH = editorH;

    if (p.x >= areaX && p.x <= areaX + areaW &&
        p.y >= areaY && p.y <= areaY + areaH)
    {
        int laneIndex = int((p.x - areaX) / laneWidth);

        if (laneIndex < 0 || laneIndex >= laneCount)
            return;

        float safeBpm = (bpm > 1.0f) ? bpm : 1.0f;
        int safeSnapDiv = (snapDiv > 1) ? snapDiv : 1;
        float beatSec = 60.0f / safeBpm;
        float snapSec = beatSec * 4.0f / safeSnapDiv;

        float centerY = editorY + editorH * 0.5f;

        float time = viewTime + (centerY - p.y) / scrollSpeed;
        time = roundf(time / snapSec) * snapSec;


        ImGuiIO& io = ImGui::GetIO();

        if (!playing && io.MouseWheel != 0.0f)
        {
            float safeBpm = (bpm > 1.0f) ? bpm : 1.0f;
            int safeSnapDiv = (snapDiv > 1) ? snapDiv : 1;
            float beatSec = 60.0f / safeBpm;
            float snapSec = beatSec * 4.0f / safeSnapDiv;

            viewTime -= io.MouseWheel * snapSec * 4.0f;

            if (viewTime < 0.0f) viewTime = 0.0f;
            if (viewTime > songLength) viewTime = songLength;

            rawPlayTime = viewTime;
            return;
        }


        if (Input::IsMouseTriggerd(0))
        {
            PushUndo();
            notes.push_back({ laneIndex, time });
            std::sort(notes.begin(), notes.end(),
                [](auto& a, auto& b) { return a.time < b.time; });

            isSaved = false;
        }
        else if (Input::IsMouseTriggerd(1))
        {
            PushUndo();
            notes.erase(
                std::remove_if(notes.begin(), notes.end(),
                    [&](auto& n)
                    {
                        return n.lane == laneIndex &&
                            fabsf(n.time - time) < snapSec * 0.5f;
                    }),
                notes.end());

            isSaved = false;
        }
    }
}

void Editor::ResetEditor()
{
    music.Stop();
    music.Seek(0.0f);

    // 再生状態
    now = 0.0f;
    start = false;
    startRequested = false;
    startTimer = 0.0f;

    playing = false;
    isSaved = true;
    rawPlayTime = 0.0f;
    viewTime = 0.0f;
    editStartTime = 0.0f;
    startTime = 0.0f;

    // 編集内容
    notes.clear();
    undoStack.clear();
    redoStack.clear();

    // 読み込み中の曲・CSV情報
    musicPath.clear();
    csvPath[0] = '\0';

    // 曲長など
    songLength = 0.0f;
}


void Editor::ResetSEFlags()
{
    for (auto& n : notes)
    {
        n.sePlayed = false;
    }
}

void Editor::Render(EditorData& data)
{
    // レーン
    for (int i = 0; i < laneCount; i++)
    {
        lane[i].SetSize(laneWidth, editorH);
        lane[i].SetPositon(Vector3(
            editorX + i * laneWidth,
            editorY,
            0
        ));
        lane[i].Render();
    }

    // レーン区切り線
    for (int i = 0; i <= laneCount; i++)
    {
        float x = editorX + i * laneWidth;

        laneLine.SetSize(4.0f, editorH);
        laneLine.SetPositon(Vector3(x, editorY, 0));
        laneLine.Render();
    }

    // 小節線・拍線
    if (bpm <= 0.0f && data.bpm > 0.0f)
    {
        bpm = data.bpm;
    }
    data.bpm = bpm;



    const int safeBeatsPerBar = (beatsPerBar > 1) ? beatsPerBar : 1;
    const float safeBpm = (bpm > 1.0f) ? bpm : 1.0f;

    const float beatSec = 60.0f / safeBpm;
    const float snapSec = beatSec * 4.0f / max(1, snapDiv);

    const float centerY = editorY + editorH * 0.5f;

    float visibleStart = viewTime - (editorH * 0.5f) / scrollSpeed;
    float visibleEnd   = viewTime + (editorH * 0.5f) / scrollSpeed;

    visibleStart = max(0.0f, visibleStart);
    visibleEnd   = min(songLength, visibleEnd);

    int startSnap = (int)std::floor(visibleStart / snapSec) - 1;
    int endSnap = (int)std::ceil(visibleEnd / snapSec) + 1;

    if (startSnap < 0) startSnap = 0;
    if (endSnap < startSnap) endSnap = startSnap;

    for (int snap = startSnap; snap <= endSnap; ++snap)
    {
        float t = snap * snapSec;
        float y = centerY - (t - viewTime) * scrollSpeed;

        if (y < editorY || y > editorY + editorH)
            continue;

        bool isBar =
            (snap % (safeBeatsPerBar * snapDiv / 4)) == 0;

        bool isBeat =
            (snap % (snapDiv / 4)) == 0;

        if (isBar)
            barlines.SetSize(editorW, 4.0f);
        else if (isBeat)
            barlines.SetSize(editorW, 2.0f);
        else
            barlines.SetSize(editorW, 1.0f);

        barlines.SetPositon(Vector3(editorX, y, 0));
        barlines.Render();
    }

    // ノーツ
    for (auto& n : notes)
    {
        float y = centerY - (n.time - viewTime) * scrollSpeed;
        if (y < editorY || y > editorY + editorH) continue;

        float x = editorX + n.lane * laneWidth;

        note.SetSize(laneWidth - 20.0f, 20.0f);
        note.SetPositon(Vector3(x + 10.0f, y - 10.0f, 0));
        note.Render();
    }

    // 判定ライン
    playhead.SetSize(editorW, 4.0f);
    playhead.SetPositon(Vector3(editorX, centerY, 0));
    playhead.Render();

}

void Editor::Release()
{
    music.Release();
   // noteSE.Release();
}


void Editor::DebugGUI()
{
#if USE_IMGUI


    
    ImGui::SetNextWindowPos(ImVec2(20, 400), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(360, 260), ImGuiCond_Always);

    ImGui::Begin(
        "Transport",
        nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize
    );

    ImGui::SetWindowFontScale(1.5f);
    ImGuiIO& io = ImGui::GetIO();

    
    

    ImGui::Separator();
    ImGui::InputFloat("BPM", &bpm);
    ImGui::InputInt("Beats/Bar", &beatsPerBar);
    if (bpm < 1.0f) bpm = 1.0f;
    if (beatsPerBar < 1) beatsPerBar = 1;

    if (ImGui::Button("Load CSV"))
    {
        char chartDir[MAX_PATH]{};

        std::filesystem::path dir =
            ProjectRoot / "Assets" / "createChart";

        strcpy_s(chartDir, dir.string().c_str());

        if (OpenFile(csvPath, MAX_PATH, "CSV\0*.csv\0", chartDir))
        {
            LoadCSV(csvPath);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Save CSV"))
    {
        char chartDir[MAX_PATH]{};

        std::filesystem::path dir =
            ProjectRoot / "Assets" / "createChart";

        strcpy_s(chartDir, dir.string().c_str());

        std::filesystem::path defaultPath =
            dir / "new_chart.csv";

        strcpy_s(csvPath, defaultPath.string().c_str());

        if (SaveFile(csvPath, MAX_PATH, "CSV\0*.csv\0", chartDir))
        {
            SaveCSV(csvPath);
        }
    }

    ImGui::End();

    // ================= BPM =================
    float beatSec = 60.0f / bpm;
    float barSec = beatSec * beatsPerBar;
    float snapSec = beatSec * 4.0f / ((snapDiv > 1) ? snapDiv : 1);

    // ================= エディター =================

    ImGui::SetNextWindowSize(
        ImVec2(520, 160),
        ImGuiCond_Always
    );
    ImGui::SetNextWindowPos(
        ImVec2(10, 100),
        ImGuiCond_Always
    );
    ImGui::Begin(
        "Chart Editor",
        nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar
    );

    ImGui::SetWindowFontScale(1.5f);
  

    if (!playing && ImGui::IsWindowHovered())
    {
        viewTime -= io.MouseWheel * snapSec * 4.0f;

        if (viewTime < 0.0f)
        {
            viewTime = 0.0f;
        }

        if (viewTime > songLength)
        {
            viewTime = songLength;
        }

        rawPlayTime = viewTime;
    }

    if (ImGui::Button("Open Music"))
    {
        LoadMusic(musicPath);
    }

    ImGui::SliderFloat("Volume", &volume, 0, 1);
    music.SetVolume(volume);

    ImGui::SliderFloat("Scroll", &scrollSpeed, 50, 600);

    const char* snapItems[] =
    {
        "1/4",
        "1/8",
        "1/12",
        "1/16",
        "1/24",
        "1/32",
        "1/1920",
        "Custom..."
    };

    const int snapValues[] =
    {
        4,
        8,
        12,
        16,
        24,
        32,
        1920
    };

    if (ImGui::Combo("Snap", &snapIndex, snapItems, IM_ARRAYSIZE(snapItems)))
    {
        if (snapIndex <= 6)
        {
            snapDiv = snapValues[snapIndex];
        }
        else
        {
            inputSnapDiv = customSnapDiv;
            ImGui::OpenPopup("Custom Snap");
        }
    }

    if (ImGui::BeginPopupModal("Custom Snap", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Custom Snap Division");
        ImGui::InputInt("1 /", &inputSnapDiv);

        if (inputSnapDiv < 1)
            inputSnapDiv = 1;

        if (ImGui::Button("OK"))
        {
            customSnapDiv = inputSnapDiv;
            snapDiv = customSnapDiv;

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (playing)
    {
        viewTime = rawPlayTime;
    }

    if (ImGui::SliderFloat("View Time", &viewTime, 0, songLength))
    {
        rawPlayTime = editStartTime + viewTime;

        music.Stop();
        music.Seek(rawPlayTime);

        if (playing)
        {
            music.Play();
        }
    }

    float progress = 0.0f;
    if (songLength > 0.0f)
    {
        progress = rawPlayTime / songLength;
    }
    progress = std::clamp(progress, 0.0f, 1.0f);

    ImGui::Separator();

    //シーク
    ImVec2 barpos = ImGui::GetCursorScreenPos();
    ImVec2 barsize = ImVec2(ImGui::GetContentRegionAvail().x, 10);

    ImGui::InvisibleButton("SeekBar", barsize);
    ImDrawList* d12 = ImGui::GetWindowDrawList();

    //見た目
    d12->AddRectFilled(barpos, ImVec2(barpos.x + barsize.x * progress,
        barpos.y + barsize.y),
        IM_COL32(100, 200, 255, 255));

    bool wasPlaying = playing;

    //クリック処理
    if (ImGui::IsItemActive())
    {
        float t = (ImGui::GetIO().MousePos.x - barpos.x) / barsize.x;
        t = std::clamp(t, 0.0f, 1.0f);

        float seektime = t * songLength;

        editStartTime = 0.0f;
        viewTime = seektime;
        rawPlayTime = seektime;
        playing = true;

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            music.Stop();
            music.Seek(rawPlayTime);

            if (playing)
            {
                music.Play();
            }

            playing = wasPlaying;
        }
    }

    // ================= SE =================
    if (playing)
    {


        const float seWindow = 0.03f;

        for (auto& n : notes)
        {
            if (n.sePlayed) continue;

            float diff = rawPlayTime - n.time;

            if (diff >= 0.0f && diff <= seWindow)
            {
                noteSE.PlaySE("SE1");
                debugSECount++;
                n.sePlayed = true;
            }

 
        }


    }

    ImGui::End();

    ImGui::Begin("DebugInfo");
    ImGui::Text("Raw GetTime() : %.3f", rawPlayTime);
    ImGui::Text("SE Count : %d", debugSECount);
    ImGui::Text("Edit Play Time : %.3f", rawPlayTime);
    ImGui::Text("Raw GetTime()  : %.3f", rawPlayTime);
    ImGui::Text("Edit Start    : %.3f", editStartTime);
    ImGui::Text("BPM = %f", bpm);

    ImGui::End();
#endif
}
