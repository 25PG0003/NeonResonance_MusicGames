#include "Scenemaingame.h"
#include "Gimmick.h"
#include "Stage.h"
#include "editor/Editor.h"
#include "RenderState.h"
#include "Input/Input.h"
#include "GameSystem.h"
#include "config/config.h"
#include "mgLib/note_data.h"

void Scenemaingame::Initialize(SceneState& scene, AudioSource& music,NoteData& notedata)
{

    // 楽曲読み込み
    music.Load("music1", "Assets/music/illusionista.wav");
    music.Load("music2", "Assets/music/koyoikimihatuki.wav", false);

    JudgeBackGround.Initialize("Assets/maingame/JudgeBackGround.png");
    JudgeBackGround.SetSize(100, 400);
    JudgeBackGround.SetPositon(Vector3(0, 85, 0));

    //ヒント描画（ノートをとる時のキー配置）
    LaneKey[0].Initialize("Assets/maingame/hintkey_S.png");
    LaneKey[1].Initialize("Assets/maingame/hintkey_D.png");
    LaneKey[2].Initialize("Assets/maingame/hintkey_J.png");
    LaneKey[3].Initialize("Assets/maingame/hintkey_K.png");
    LaneKey[4].Initialize("Assets/maingame/hintkey_L.png");

    for (int i = 0; i < 5; i++) {
        LaneKey[i].SetSize(100, 100);

    }

    /*LaneKey[1].SetPositon(Vector3(notedata.LaneKeyX[1], 900, 0));
    LaneKey[2].SetPositon(Vector3(notedata.LaneKeyX[2], 900, 0));
    LaneKey[3].SetPositon(Vector3(notedata.LaneKeyX[3], 900, 0));
    LaneKey[4].SetPositon(Vector3(notedata.LaneKeyX[4], 900, 0));
    LaneKey[0].SetPositon(Vector3(notedata.LaneKeyX[0], 900, 0));*/

    AutoModeHint.Initialize("Assets/maingame/automode.png");
    AutoModeHint.SetSize(300, 300);
    AutoModeHint.SetPositon(Vector3(1500, 100, 0));

    Perfect_Sprite.Initialize("Assets/judge/perfect.png");
    Perfect_Sprite.SetSize(100, 100);
    Perfect_Sprite.SetPositon(Vector3(0, 185, 0));

    Good_Sprite.Initialize("Assets/judge/Good.png");
    Good_Sprite.SetSize(100, 100);
    Good_Sprite.SetPositon(Vector3(0, 285, 0));

    Miss_Sprite.Initialize("Assets/judge/miss.png");
    Miss_Sprite.SetSize(100, 100);
    Miss_Sprite.SetPositon(Vector3(0, 385, 0));

    ComboSprite.Initialize("Assets/judge/Combo.png");
    ComboSprite.SetSize(100, 100);
    ComboSprite.SetPositon(Vector3(0, 85, 0));

    SpeedChangeHint.Initialize("Assets/maingame/SpeedChangeHint.png");
    SpeedChangeHint.SetSize(400, 220);
    SpeedChangeHint.SetPositon(Vector3(1400, 400, 0));

    ResultChangeHint.Initialize("Assets/maingame/ResultSceneChangeHint.png");
    ResultChangeHint.SetSize(200, 400);
    ResultChangeHint.SetPositon(Vector3(550, 400, 0));

    //譜面と音楽再生ボタン
    StartButton.Initialize("Assets/maingame/pressspace.png");
    StartButton.SetSize(800, 400);
    StartButton.SetPositon(Vector3(550, 400, 0));

    RestartHint.Initialize("Assets/maingame/ResetHint.png");
    RestartHint.SetSize(200, 100);
    RestartHint.SetPositon(Vector3(100, 600, 0));

    //開始合図画像読み込み
    Num[0].Initialize("Assets/maingame/num_1.png");
    Num[1].Initialize("Assets/maingame/num_2.png");
    Num[2].Initialize("Assets/maingame/num_3.png");

    for (int i = 0; i < 3; i++) {
        Num[i].SetSize(100, 100);
        Num[i].SetPositon(Vector3(630, 200, 0));
    }

    IsLaneKey = true;
}

void Scenemaingame::ResetGame(Editor& editor,GameTime& gameTime,Gimmick& gimmick, AudioSource& music)
{
    music.Stop("music2");
    music.Seek("music2", 0.0f);
    editor.now = 0.0f;

    editor.start = false;
    editor.startRequested = false;
    editor.startTimer = 0.0f;

    showResultchangeHint(false);

    gameTime.Reset();
    gimmick.ClearRelease();
}

void Scenemaingame::LoadMusicFromChart(Gimmick& gimmick,AudioSource& music)
{
    music.Stop("music2");
    music.ReleaseVoice("music2");

    if (!gimmick.GetMusicPath().empty())
    {
        music.Load("music2", gimmick.GetMusicPath().c_str(), false);
    }
    else
    {
        music.Load("music2", "Assets/music/koyoikimihatuki.wav");
    }
}

void Scenemaingame::Update(
    SceneState& scene, Gimmick& gimmick, Editor& editor,
    GameTime& gameTime, Player& pl, Stage& stage, 
    AudioSource& music)
{
    if (Input::IsKeyTriggerd('R'))
    {
        ResetGame(editor,gameTime,gimmick,music);
        return;
    }

    if (!editor.start && Input::IsKeyTriggerd(VK_SPACE)) //Spaceを押すとリクエストがtrueになる
    {
        editor.startRequested = true;
        editor.startTimer = 0.0f;
    }

    if (Input::IsKeyTriggerd(VK_ESCAPE))
    {
        music.Stop("music2");
        music.Seek("music2", 0.0f);

        editor.now = 0.0f;
        editor.start = false;
        editor.startRequested = false;
        editor.startTimer = 0.0f;
        showResultchangeHint(false);

        gameTime.Reset();
        scene.Set(SELECTMODE);
    }

    //Spaceを押すとリクエストがtrueになり、ゲームが開始する
    if (editor.startRequested && !editor.start)
    {
        editor.startTimer += DeltaTime;
        if (editor.startTimer >= startDelay)
        {
            editor.start = true;
            gameTime.Start();
            music.Play("music2");
        }
    }

    // レーンキー位置更新
    float rate = 6.0f / Config::LaneAngle;

    for (int i = 0; i < 5; i++)
    {
        float x =
            900.0f +
            (BaseLaneKeyX[i] - 900.0f) * rate;

        LaneKey[i].SetPositon(
            Vector3(x, 900, 0)
        );
    }

    if (Input::IsKeyTriggerd('Q'))
    {
        gimmick.UpdateScoreResult();
        ResetGame(editor,gameTime,gimmick,music);
        scene.Set(Result);
    }
    //時間更新
    gameTime.Update();

    if (!editor.start) {
        editor.now = 0.0f;
    }
    else {
        //時間取得
        editor.now = music.GetPlayTime(std::string("music2")) * speedRate;
    }
    //ノート更新
    gimmick.Update(editor.now);

    //プレイヤー更新
    pl.Update(editor.now);


    //ステージ更新
    stage.Update(gameTime);




    if (editor.start)
    {
        if (!music.IsPlay("music2") && gimmick.IsAllNotesCleared())
        {
            gimmick.UpdateScoreResult();

            showResultchangeHint(true);
            scene.Set(Result);
        }


        //Uキーでオートモード起動（切り替え可）
        if (Input::IsKeyTriggerd('U'))
        {
            gimmick.SetAutoPlay(!gimmick.IsAutoPlay());
        }

        gimmick.JudgeAuto(editor.now);
        //レーン番号
        int lane = -1;

        //ノートの判定
        while (pl.ConsumeAttack(lane))
        {
            if (lane >= 0 && lane < 5)
            {
                gimmick.JudgeAttack(editor.now+0.01f, lane);
            }
        }
    }
}

void Scenemaingame::Release()
{
    //曲の全開放
    music.ReleaseAllVoice();
}

void Scenemaingame::Render(SceneState& scene, Stage& stage, Gimmick& gimmick, Editor& editor)
{

    //ステージ有効
    stage.Draw();

    //ノート描画
    gimmick.Draw();

    JudgeBackGround.Render();

    if (IsLaneKey)
    {
        for (int i = 0; i < 5; i++)
        {
            LaneKey[i].Render();
        }
    }

    //Depth無効
    RenderState::DepthEnable(false);

    AutoModeHint.Render();
    gimmick.SetComboRender(true);

    Perfect_Sprite.Render();
    Good_Sprite.Render();
    Miss_Sprite.Render();
    ComboSprite.Render();
    SpeedChangeHint.Render();

    if (showResultChangeHint)
    {
        ResultChangeHint.Render();
    }

    //startがtrueになったら描画を止める。falseの場合は継続
    if (!editor.start)
    {
        //スタートボタン描画
        StartButton.Render();
        RestartHint.Render();
    }

    if (editor.startRequested)
    {
        if (editor.startTimer < 1.0f)
        {
            Num[2].Render();
        }
        else if (editor.startTimer < 2.0f)
        {
            Num[1].Render();
        }
        else if (editor.startTimer < 3.0f)
        {
            Num[0].Render();
        }

    }
}
