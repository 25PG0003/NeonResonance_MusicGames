#pragma once
#include <filesystem>
#include "../FbxMesh.h"
#include "../AudioSystem/AudioSource.h"
#include "../musicdata.h"
#include "../config/config.h"
#include <string>

class NoteData
{
private:

	//オートプレイか？
	bool AutoPlay = false;

	//late・fastカウント
	int FastCount = 0;
	int LateCount = 0;

	//コンボ数・perfect・good・miss数
	int PerfectNum = 0;
	int GoodNum = 0;
	int MissNum = 0;
	int ComboNum = 0;

	//スコア・最大スコア・総ノーツ数
	int score = 0;
	int maxscore = 1000000;

	int totalNotes = 0;

	//判定幅
	const float PERFECT_Z = 0.040f;
	const float GOOD_Z = 0.100f;
	const float miss_window = 1.2f;  // judgeZ を越えたらMISS

	//FULLCOMBO/ALLPERFECT
	bool fullCombo = false;
	bool allPerfect = false;


	MusicData data;

	


	//ノーツデータ
	struct Note
	{
		float time;
		int lane;
		int type;
	};

	struct ActiveNote
	{
		Note note;
		FBXMesh* mesh;
		bool alive = true;
		bool judged = false;
		float z;
	};

	struct JudgeEffect
	{
		int type;
		float timer;
		Vector3 position;
	};

	struct BarLine
	{
		float time;
		FBXMesh* mesh;
	};

	//SE再生
	AudioSource SE;

	//小節線セット
	const int BEATS_PER_BAR = 4;
	
	float nextBarTime = 0.0f;

	//ノーツデータのインデックス
	size_t nextNoteIndex = 0;

	//Z判定
	const float judgeZ = 1.5f;

	
	//ノーツ出現レーン座標
	const float LaneX[5] = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f };


	//ノーツ生成Z座標
	float SpawnZ = -50.0f;

	//？
	float BestDiff = 0.0f;
	int writeIndex = 0;

public:
	NoteData();
	~NoteData();

	void Initialize();
	void Release();
	void Update(float deltaTime);
	void Render();

	void ResetData();
	void ClearRelease();
	void SERelease();
	void ResetResult();
	void UpdateScoreResult();

	//ノーツ読み込み
	void SetNoteData(const std::string& filepath);
	
	//判定
	void JudgeAttack(float musicTime, int attackLane);

	//オートプレイ設定
	void SetAutoPlay(bool enable);
	bool IsAutoPlay()const { return AutoPlay; }
	void JudgeAuto(float musicTime);

	//ノーツ表示開始位置設定
	void UpdateSpawnZ();

	//Perfect/Good/Miss/Combo/Late/Fast/CountGetter
	int GetFastCount()const { return FastCount; }
	int GetLateCount()const { return LateCount; }
	int GetPerfectCount()const { return PerfectNum; }
	int GetGoodCount()const { return GoodNum; }
	int GetMissCount()const { return MissNum; }
	int GetComboCount()const { return ComboNum; }

	//スコア・最大スコア・総ノーツ数Getter
	int GetScore() const { return score; }
	int GetMaxScore() const { return maxscore; }
	int GetTotalNotes() const { return totalNotes; }

	bool IsFullCombo() const { return fullCombo; }
	bool IsAllPerfect() const { return allPerfect; }

	//ノーツ生成
	void SpawnNote(float musicTime);

	// Preview用ノーツ生成
	void CreatePreviewNotes();

	// Preview用：一番判定ラインに近いノーツを取る
	void JudgePreview(float musicTime);

	//小節線生成
	void SpawnBarLines(float musicTime);

	//スピードセット
	void SetSpeed(float speed);
	void UpdateSpeed();

	//Bpm取得
	float GetBPM()const { return data.bpm; }

	//判定エフェクト
	std::vector<JudgeEffect> judgeEffectQueue;
	//ノーツデータ
	std::vector<ActiveNote> activeNotes;
	//ノーツデータ
	std::vector<Note> notes;
	//？
	std::vector<FBXMesh*> verticalline;

	std::string musicPath;

	std::vector<BarLine> barLines;

	//画面X座標(エフェクト用)
	float screenX[5] = { 450, 650, 850, 1050, 1250 };

	float LaneKeyX[5] = { 600.0f,
	750.0f,
	900.0f,
	1050.0f,
	1200.0f };

	bool IsAllNotesCleared()const { return activeNotes.empty(); }
};