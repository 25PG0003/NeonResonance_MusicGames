#pragma once
#include "Scene/SceneState.h"
#include "Render2D.h"
#include "CommonUI.h"
#include "Gimmick.h"
#include <string>
#include "Scenemaingame.h"
#include "AudioSystem/AudioSource.h"
#include "editor/Editor.h"

#include <fstream>
#include <sstream>
#include <windows.h>
#include "musicdata.h"

#include "editor/Editor.h"
#include <vector>
#include <string>
enum SelectStep
{
	SelectSong,
	SelectDifficulty,
};

class SceneSelectMusic
{
public:
	void Initialize(SceneState& scene);
	void Update(SceneState& scene,Gimmick& gimmick,AudioSource& music, Scenemaingame& maingame,Editor& editor,Editor::EditorData& data);
	void Release(SceneState& scene);
	void Render(SceneState& scene);

	bool IsLoadChart() const { return loadChart; }
	const std::string& GetSelectedChart() const { return selectedChart; }
	void ResetLoadChart() { loadChart = false; }
	void LoadChartList();

	bool HasChart(int musicIndex, int difficulty) const;
	bool HasAnyChart(int musicIndex) const;

private:


	UI ui;

	Render2D musicselecthint;
	Render2D Jacketholder[3]; //仮で３つ
	Render2D currentjacket;

	//ジャケ絵
	Render2D koyoi_jacket;
	Render2D hydro_jacket;

	SelectStep step = SelectSong;
	std::string chartList[3][3];

	bool loadChart = false;
	std::string selectedChart;


	

	std::vector<MusicData> musicList;
	std::vector<Render2D*> jacketImages;
	


	int currentmusic = 0;
	int currentDifficulty = 0;
	int lastPreviewMusic = -1;
	int loadedPreviewMusic = -1;


	bool showNotChartPopup = false;
	bool editorMode = false;
	
};

