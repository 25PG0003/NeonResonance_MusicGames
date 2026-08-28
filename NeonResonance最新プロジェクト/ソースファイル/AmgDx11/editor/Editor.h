#pragma once
#include "../time.h"
#include "../AudioSystem/AudioVoice.h"

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

// ===== Windows =====
#include <windows.h>
#include <shobjidl.h>
#include <propsys.h>
#include <propkey.h>
#include <propvarutil.h>
#include "../AudioSystem/AudioSource.h"
#include "../Render2D.h"
#include"../musicdata.h"
#include "../Gimmick.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "propsys.lib")


class Editor
{
public:

    // =======================
    // 譜面データ
    // =======================
    struct ChartNote
    {
        int lane = 0;

        float time = 0.0f;

        float duration = 0.0f;  // ← ロングノーツの長さ（秒）

        bool sePlayed = false;
    };

    // =======================
    // Undo Snapshot
    // =======================
    struct NotesSnapshot
    {
        std::vector<ChartNote> notes;
    };

    //スタートボタンフラグ
    bool start; //スタートしたか？
    //3秒後開始
    bool startRequested = false;
    float startTimer = 0.0f;
    float now;

    bool isSaved = true;

    struct EditorData
    {
        std::string musicPath;
        float bpm = 120.0f;
    };


private:

    //２D画像読み込み
    Render2D note;
    Render2D lane[5];
    Render2D barlines;
    Render2D playhead;
    Render2D laneLine;

    // =======================
    // Audio
    // =======================
    Voice music;

    AudioSource noteSE;

    // =======================
    // Notes
    // =======================
    std::vector<ChartNote> notes;

    // =======================
    // Undo / Redo
    // =======================
    std::vector<NotesSnapshot> undoStack;

    std::vector<NotesSnapshot> redoStack;


    // =======================
    // State
    // =======================
    bool playing = false;

    float bpm = 120.0f;
    int beatsPerBar = 4;
    int snapDiv = 4;

    float songLength = 120.0f;
    float viewTime = 0.0f;

    float scrollSpeed = 200.0f;
    float volume = 1.0f;

    std::string musicPath;
    char csvPath[MAX_PATH]{};

    float prevPlayTime = 0.0f;

    // ===== 追加：編集基準スタート時間 =====
    float editStartTime = 0.0f;

    float rawPlayTime;

    float startTime = 0.0f;

    const int laneCount = 5;
    const float laneWidth = 120.0f;
    const float canvasHeight = 900.0f;
    float offset = 0.0f;

    int debugSECount = 0;

    float editorX = 640.0f;
    float editorY = 0.0f;
    float editorW = 600.0f;
    float editorH = 1200.0f;

    int snapIndex = 0;

    int customSnapDiv = 64;
    int inputSnapDiv = 64;

    // ロングノーツ配置用
    bool longNoteMode = false;        // ロングノーツ配置モード
    float longNoteStartTime = -1.0f;  // ロングノーツ開始時刻
    int longNoteLane = -1;            // ロングノーツのレーン




public:

    // =======================
    // 基本
    // =======================
    bool Initialize();

    void Release();

    void Update();

    void Render(EditorData& data);

    void DebugGUI();

    // =======================
    // Undo / Redo
    // =======================
    void PushUndo();

    void Undo();

    void Redo();

    // =======================
    // Save / Load
    // =======================
    void SaveCSV(const char* path);

    void LoadCSV(const char* path);

    void SaveChart();

    void LoadMusic(const std::string& path);

    void LoadFromGimmick(const EditorData& data);

    //エディター内機能関数
    void ResetSEFlags();

    void ResetEditor();



    bool IsSaved() const { return isSaved; }
    void SetUnsaved() { isSaved = false; }



};
