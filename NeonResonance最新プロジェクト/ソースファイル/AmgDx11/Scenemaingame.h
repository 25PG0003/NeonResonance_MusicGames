#pragma once

#include "Scene/SceneState.h"
#include "Gimmick.h" 
#include "Stage.h"
#include "Render2D.h"
#include "editor/Editor.h"
#include "AudioSystem/AudioSource.h"
#include "Player.h"
#include "mgLib/note_data.h"

// キー画像の基準X座標
static const float BaseLaneKeyX[5] =
{
	600.0f,
	700.0f,
	900.0f,
	1000.0f,
	1200.0f
};

class Scenemaingame
{
public:
	void Initialize(SceneState& scene,AudioSource& music, NoteData& notedata);
	void Update(SceneState& scene, Gimmick& gimmick, Editor& editor, GameTime& gameTime, Player& pl,Stage& stage, AudioSource& music);
	void Release();
	void Render(SceneState& scene, Stage& stage, Gimmick& gimmick,Editor& editor);

	bool showResultchangeHint(bool a) { showResultChangeHint = a; return showResultChangeHint; }
	void ResetGame(Editor& editor,GameTime& gameTime, Gimmick& gimmick, AudioSource& music);

	void LoadMusicFromChart(Gimmick& gimmick,AudioSource& music);
private:


	//LaneKeyを描画するかどうか
	bool IsLaneKey;
	bool showResultChangeHint = false;

	const float startDelay = 3.0f;

	float speedRate = 1.0f;

	//Startボタン画像
	Render2D StartButton;

	//ヒントキー画像読み込み
	Render2D LaneKey[5];

	//数字読み込み
	Render2D Num[3];

	Render2D Perfect_Sprite;
	Render2D Good_Sprite;
	Render2D Miss_Sprite;
	Render2D ComboSprite;
	//autoモード起動ヒント画像
	Render2D AutoModeHint;
	//リザルト遷移ヒント
	Render2D ResultChangeHint;
	//スピードチェンジのヒント画像
	Render2D SpeedChangeHint;
	//メインゲームリスタート用ボタン画像
	Render2D RestartHint;
	Render2D JudgeBackGround;

	AudioSource music;
};

