#pragma once
#include "Scene/SceneState.h"
#include "Render2D.h"
#include "CommonUI.h"
#include "RenderState.h"
#include "AudioSystem/AudioSource.h"
#include "Gimmick.h"
#include "Scenemaingame.h"
#include <cmath>

class SelectMode
{
public:
	SelectMode() {};
	~SelectMode() {};

	bool Initialize(SceneState& scene);
	void Update(SceneState& scene, AudioSource* audio,Gimmick& gimmick,Scenemaingame& maingame);
	void Release();
	void Render(SceneState& scene);
	std::string OpenChartFile();
	bool IsLoadChart() const { return loadchart; }
	void SetLoadChart(bool load) { loadchart = load; }
	void ResetLoadChart() { loadchart = false; }
	const std::string& GetSelectedChartPath() const;

	//イージング関数
	float EaseOutCubic(float t);


private:
	//難易度選択画面
	Render2D difficultScene;
	

	//マウス対応
	Render2D SceneFreePlay[2];
	Render2D SceneChartLoad[2];
	Render2D SceneEditor[2];
	Render2D SceneTitle[2];

	Render2D optionHint[2];

	//共通画像
	UI ui;

	//楽曲変数
	AudioSource* music = nullptr;

	bool chartLoaded = false;
	bool openLoadSuccessPopup = false;
	bool openLoadFailedPopup = false;
	bool loadchart = false;
	bool showChartSelect = false;

	int selectedChartIndex = -1;

	int selectedMenu = 0;

	//イージング用
	float menuAnim[4] = {};


	// SelectMode.h
	std::string selectedChartPath;

};

