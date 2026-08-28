#pragma once
#include <vector>
#include "FBXMesh.h"
#include "Render2D.h"
#include "AudioSystem/AudioSource.h"
#include "mgLib/note_data.h"
#include "musicdata.h"

class Gimmick
{
public:
    Gimmick();
    ~Gimmick();

    void Initialize();
    void Release();
    void SERelease();
   

    void LoadCSV(const std::string& filepath);
    void ComboUpdate(float x, float y, int num);
    void Update(float dt);
    void Draw();
    void JudgeAuto(float musicTime);
    void SpawnBarLines(float musicTime);
    void SetAutoPlay(bool enable);
    void CreatePreviewNotes();
    void JudgePreview(float musicTime);
    void SetSpeed(float speed);
    void SetComboRender(bool Combo) { ComboRender = Combo; }
    void ClearRelease();
    void JudgeAttack(float musicTime, int lane);
    void SetJacket(const char* jacket, int width, int height, float x, float y, float z);
    void Debuginfo();
    void UpdateScoreResult();
    void ResetResult();

    // 判定ラインの色を変更
    void SetJudgeLineColor(float r, float g, float b, float alpha = 1.0f);
 
    bool IsAutoPlay()const { return autoPlay; }
    bool IsAllNotesCleared()const;

    float GetBPM()const { return notedata.GetBPM(); }
    float GetSpeed()const;

    const std::string& GetMusicPath()const { return notedata.musicPath; }

    NoteData notedata;
private:

    bool ComboRender = false;

    MusicData data;

    float bpm = 120.0f;
    float offset = 0.0f;

    Render2D Perfect;
    Render2D Good;
    Render2D Miss;

    Render2D ComboNum;

    Render2D perfectEffect;
    Render2D goodEffect;
    Render2D missEffect;

    bool autoPlay = false;

    // 判定ラインの設定
    struct JudgeLineConfig
    {
        Vector3 color = Vector3(1.0f, 1.0f, 0.0f);  // 黄色（デフォルト）
        float alpha = 1.0f;
    } judgeLineConfig;

    const float PERFECT_Z = 0.035f;
    const float GOOD_Z = 0.080f;
    const float miss_window = 1.2f;  // judgeZ を越えたらMISS

    


    
};
