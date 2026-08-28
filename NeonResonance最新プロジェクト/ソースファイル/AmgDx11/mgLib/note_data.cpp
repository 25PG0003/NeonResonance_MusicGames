#include "note_data.h"
#include "../musicdata.h"
#include "../config/config.h"
#include "../Input/Input.h"
#include <Windows.h>

//デルタタイム用
#include "../GameSystem.h"

//ファイル読み込み関連
#include <string>
#include <fstream>
#include <sstream>


NoteData::NoteData(){}
NoteData::~NoteData() { Release(); }

//初期化
void NoteData::ResetData()
{
    activeNotes.clear();
    barLines.clear();
    nextNoteIndex = 0;
    nextBarTime = 0.0f;
    

    ComboNum = 0;
    PerfectNum = 0;
    GoodNum = 0;
    MissNum = 0;

    LateCount = 0;
    FastCount = 0;
    score = 0;

    totalNotes = 0;
    fullCombo = false;
    allPerfect = false;

    notes.clear();
}

void NoteData::Initialize()
{
    ResetData();
    //SE読み込み
    SE.Load("PerfectSE", "Assets/SE/perfect.wav");
    SE.Load("GoodSE", "Assets/SE/good.wav");
    SE.Load("MissSE", "Assets/SE/miss.wav");

    SE.Volume("PerfectSE", 1.0f);
    SE.Volume("GoodSE", 1.0f);
    SE.Volume("MissSE", 1.0f);

    //レーン読み込み
    for (auto i = 0; i < 6; i++) {
        auto mesh = new FBXMesh();
        mesh->Create("Assets/gimmick/laneline.fbx");
        mesh->scale = Vector3(0.5f, 0.2f, 10.0f);
        mesh->position = Vector3(0.0f, -0.5f, 0.0f);
        verticalline.push_back(mesh);
    }

    std::sort(notes.begin(), notes.end(),
        [](const Note& a, const Note& b)
        {
            return a.time < b.time;
        });
}

void NoteData::Update(float deltaTime)
{
  // ------------------------------
  // 小節線更新
  // ------------------------------
    writeIndex = 0;
    for (int i = 0; i < barLines.size(); ++i)
    {
        BarLine& b = barLines[i];
        float dt = b.time - deltaTime;
        b.mesh->position.z = dt * Config::notespeed;
        b.mesh->Update();

        // 判定ラインを越えたら削除
        if (b.mesh->position.z >= judgeZ)
        {
            delete b.mesh;
            continue;
        }

        barLines[writeIndex++] = b;
    }
    barLines.resize(writeIndex);

    // ------------------------------
    // ノーツ更新
    // ------------------------------
    writeIndex = 0;
    for (int i = 0; i < activeNotes.size(); ++i)
    {
        ActiveNote& n = activeNotes[i];
        if (!n.mesh) continue;

        // スクロール更新

        float travelTime = fabs(judgeZ - SpawnZ) / fabs(Config::notespeed);

        float t = 1.0f - ((n.note.time - deltaTime) / travelTime);

        t = std::clamp(t, 0.0f, 1.0f);

        n.z = SpawnZ + (judgeZ - SpawnZ) * t;



        n.mesh->position.z = n.z;
        n.mesh->Update();


        // alive/judged ノーツは削除
        if (!n.alive || n.judged)
        {
            delete n.mesh;
            continue;
        }

        if (n.alive && !n.judged && (deltaTime - n.note.time) > miss_window)
        {
            n.alive = false;
            n.judged = true;
            MissNum++;

            Vector3 missPos(screenX[n.note.lane], 600, 0);
            judgeEffectQueue.push_back({ 2, 0.1f, missPos });
            judgeEffectQueue.push_back({ 12, 0.2f, missPos });

            ComboNum = 0;
            SE.PlaySE("MissSE");
        }

        if (n.alive && !n.judged)
        {
            activeNotes[writeIndex++] = n;
        }
        else
        {
            delete n.mesh;
        }
    }
    activeNotes.resize(writeIndex);



    // 判定エフェクトキューの更新
    writeIndex = 0;
    for (int i = 0; i < judgeEffectQueue.size(); i++)
    {
        JudgeEffect& e = judgeEffectQueue[i];
        e.timer -= DeltaTime;

        if (e.timer > 0.0f)
        {
            judgeEffectQueue[writeIndex++] = e;
        }
    }
    judgeEffectQueue.resize(writeIndex);
}

void NoteData::ClearRelease()
{
    for (auto& n : activeNotes)
    {
        delete n.mesh;
        n.mesh = nullptr;
    }
    activeNotes.clear();

    nextNoteIndex = 0;

    ComboNum = 0;
    LateCount = 0;
    FastCount = 0;

    totalNotes = 0;
    fullCombo = false;
    allPerfect = false;
}

void NoteData::Release()
{
    for (auto& b : barLines)
    {
        delete b.mesh;
        b.mesh = nullptr;
    }
    barLines.clear();

    for (auto& n : activeNotes)
        delete n.mesh;
    activeNotes.clear();

    for (auto& v : verticalline)
    {
        delete v;
        v = nullptr;
    }
    verticalline.clear();
    nextNoteIndex = 0;
    nextBarTime = 0.0f;
}

void NoteData::SERelease()
{
    SE.ReleaseAllVoice();
}

void NoteData::ResetResult()
{
    score = 0;
    ComboNum = 0;
    PerfectNum = 0;
    GoodNum = 0;
    MissNum = 0;
    FastCount = 0;
    LateCount = 0;
    fullCombo = false;
    allPerfect = false;
}

void NoteData::JudgeAuto(float musicTime)
{
    //オートモードじゃなければ
    if (!AutoPlay)return;

    //perfect判定
    const float PERFECT = 0.030f;


    for (int i = 0; i < activeNotes.size(); i++)
    {
        auto& n = activeNotes[i];

        if (!n.alive || n.judged)continue;

        float diff = fabs(n.note.time - musicTime);

        if (diff <= PERFECT)
        {
            JudgeAttack(musicTime, n.note.lane);
        }
    }
}

void NoteData::SetAutoPlay(bool enable)
{
    AutoPlay = enable;
}

void NoteData::UpdateSpawnZ()
{
    if (Input::IsKeyTriggerd('5'))
    {
        SpawnZ -= 1.0f;
        if (SpawnZ < -50.0f)
        {
            SpawnZ = -50.0f;
        }
    }
    if (Input::IsKeyTriggerd('4'))
    {
        SpawnZ += 1.0f;
        if (SpawnZ > -10.0f)
        {
            SpawnZ = -10.0f;
        }
    }
}


void NoteData::UpdateScoreResult()
{
    if (totalNotes <= 0)
    {
        score = 0;
        fullCombo = false;
        allPerfect = false;
        return;
    }

    float rate =
        (PerfectNum * 1.0f + GoodNum * 0.7f)
        / totalNotes;

    score = (int)(rate * maxscore);

    fullCombo = (MissNum == 0);
    allPerfect = (PerfectNum == totalNotes);

    if (allPerfect)
    {
        score = maxscore;
    }
}

void NoteData::SetSpeed(float speed)
{
    Config::notespeed = speed;
}

void NoteData::UpdateSpeed()
{
  /*  if (Input::IsKeyTriggerd('9'))
    {
        Config::notespeed -= 1.0f;
        if (Config::notespeed < -20.0f)
        {
            Config::notespeed = -20.0f;
        }
    }
    if (Input::IsKeyTriggerd('8'))
    {
        Config::notespeed += 1.0f;
        if (Config::notespeed > -1.0f)
        {
            Config::notespeed = -1.0f;
        }
    }*/
}

void NoteData::SpawnBarLines(float musicTime)
{
    const float beatTime = 60.0f / data.bpm;
    const float barTime = beatTime * BEATS_PER_BAR;
    const float appearTime = 2.0f;

    while (nextBarTime - musicTime <= appearTime)
    {
        FBXMesh* mesh = new FBXMesh();
        mesh->Create("Assets/gimmick/barline.fbx");
        mesh->scale = Vector3(3.0f, 0.3f, 0.3f);
        mesh->position = Vector3(0.0f, -0.28f, 0.0f);

        barLines.push_back({ nextBarTime, mesh });
        nextBarTime += barTime;
    }
}

void NoteData::Render()
{
   
}

void NoteData::SpawnNote(float musicTime)
{
    const float appearTime = fabs(judgeZ - SpawnZ) / fabs(Config::notespeed);


    while (nextNoteIndex < notes.size())
    {
        Note& n = notes[nextNoteIndex];

        if (n.time - musicTime > appearTime)
            break;

        FBXMesh* mesh = new FBXMesh();
        mesh->Create("Assets/gimmick/box.fbx");
        mesh->scale = Vector3(0.5f, 0.5f, 0.5f);

        mesh->position = Vector3(LaneX[n.lane], 0.0f, SpawnZ);

        mesh->Update();

        activeNotes.push_back({ n,mesh });
        nextNoteIndex++;
    }
}

void NoteData::JudgeAttack(float musicTime, int attackLane)
{
    int targetIndex = -1;
    float bestDz = GOOD_Z;

    for (int i = 0; i < activeNotes.size(); ++i)
    {
        auto& n = activeNotes[i];

        if (!n.mesh) continue;
        if (n.note.lane != attackLane) continue;
        if (!n.alive || n.judged) continue;

        float diff = (n.note.time - musicTime + Config::Offset);
        if (fabs(diff) <= GOOD_Z && fabs(diff) < bestDz)
        {
            bestDz = fabs(diff);
            BestDiff = diff;
            targetIndex = i;
        }
    }

    if (targetIndex < 0) return;

    if (bestDz <= PERFECT_Z)
    {
        judgeEffectQueue.push_back({ 0, 0.1f, Vector3(screenX[attackLane], 600, 0) });
        judgeEffectQueue.push_back({ 10, 0.2f, Vector3(screenX[attackLane], 600, 0) });

        PerfectNum++;
        ComboNum++;
        score += 1000;
        SE.PlaySE("PerfectSE");
    }
    else
    {
        judgeEffectQueue.push_back({ 1, 0.1f, Vector3(screenX[attackLane], 600, 0) });
        judgeEffectQueue.push_back({ 11, 0.2f, Vector3(screenX[attackLane], 600, 0) });

        GoodNum++;
        ComboNum++;
        score += 500;
        SE.PlaySE("GoodSE");

        if (BestDiff >= 0.0f)
            FastCount++;
        else
            LateCount++;
    }

    auto& n = activeNotes[targetIndex];
    n.alive = false;
    n.judged = true;
}

void NoteData::CreatePreviewNotes()
{
    // 現在のノーツを消す
    for (auto& n : activeNotes)
    {
        delete n.mesh;
        n.mesh = nullptr;
    }

    activeNotes.clear();

    notes.clear();

    nextNoteIndex = 0;

    // Preview用BPM
    data.bpm = 120.0f;

    // 1拍 = 60 / BPM
    const float beatTime = 60.0f / data.bpm;

    // 4/4拍子なので1小節 = 4拍
    const float barTime = beatTime * BEATS_PER_BAR;

    // 30小節分
    const int previewBars = 30;

    for (int i = 0; i < previewBars; i++)
    {
        Note n{};

        // 1小節に1ノーツ
        n.time = (i + 1) * barTime;

        // S → D → J → K → L → S...
        n.lane = i % 5;

        // Tapノーツ
        n.type = 0;

        notes.push_back(n);
    }

    totalNotes = static_cast<int>(notes.size());
}

void NoteData::JudgePreview(float musicTime)
{
    int targetIndex = -1;
    float bestDiff = GOOD_Z;

    for (int i = 0; i < activeNotes.size(); i++)
    {
        auto& n = activeNotes[i];

        if (!n.mesh) continue;
        if (!n.alive || n.judged) continue;

        float diff = fabs(n.note.time - musicTime);

        if (diff <= GOOD_Z && diff < bestDiff)
        {
            bestDiff = diff;
            targetIndex = i;
        }
    }

    if (targetIndex < 0)
        return;

    auto& n = activeNotes[targetIndex];

    // Perfect
    if (bestDiff <= PERFECT_Z)
    {
        n.alive = false;
        n.judged = true;
    }
    // Good
    else
    {
        n.alive = false;
        n.judged = true;
    }
}

void NoteData::SetNoteData(const std::string& filepath)
{
    std::string cwd = std::filesystem::current_path().string();
    notes.clear();
    ResetResult();

    for (auto& n : activeNotes)
    {
        delete n.mesh;
    }

    activeNotes.clear();

    musicPath.clear();
    nextNoteIndex = 0;

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        OutputDebugStringA("Gimmick LoadCSV open failed: ");
        OutputDebugStringA(filepath.c_str());
        OutputDebugStringA("\n");
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        if (line.size() >= 3 &&
            (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB &&
            (unsigned char)line[2] == 0xBF)
        {
            line.erase(0, 3);
        }

        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (line.empty()) continue;

        if (line[0] == '#')
        {
            std::stringstream ss(line.substr(1));
            std::string key, value;

            std::getline(ss, key, ',');
            std::getline(ss, value);

            OutputDebugStringA(("KEY=[" + key + "] VALUE=[" + value + "]\n").c_str());

            // key の空白除去
            while (!key.empty() && key.front() == ' ')
                key.erase(0, 1);
            while (!key.empty() && key.back() == ' ')
                key.pop_back();

            // value の空白除去
            while (!value.empty() && value.front() == ' ')
                value.erase(0, 1);
            while (!value.empty() && value.back() == ' ')
                value.pop_back();

            if (key == "BPM")
            {
                data.bpm = std::stof(value);
            }
            else if (key == "OFFSET")
            {
                data.offset = std::stof(value);
            }
            else if (key == "MUSIC")
            {
                musicPath = value;
            }

            continue;
        }

        // =========================
        // ノーツ読み込み
        // =========================
        std::stringstream ss(line);
        std::string item;
        Note n{};

        std::getline(ss, item, ',');
        n.time = std::stof(item);

        std::getline(ss, item, ',');
        n.lane = std::stoi(item);

        std::getline(ss, item, ',');
        n.type = std::stoi(item);

        if (n.lane < 0 || n.lane >= 5)
            continue;

        notes.push_back(n);
    }

    std::sort(notes.begin(), notes.end(),
        [](const Note& a, const Note& b)
        {
            return a.time < b.time;
        });

    nextNoteIndex = 0;

    for (auto& n : activeNotes)
    {
        delete n.mesh;
    }
    activeNotes.clear();

    totalNotes = (int)notes.size();

    OutputDebugStringA("Gimmick final musicPath = ");
    OutputDebugStringA(musicPath.c_str());
    OutputDebugStringA("\n");
}