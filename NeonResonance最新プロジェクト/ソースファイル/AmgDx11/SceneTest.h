// sceneTest.h
#pragma once
#include "CommonUI.h"
#include "DxSystem.h"
#include "DXShader.h"
#include "FBXMesh.h"
#include "Stage.h"
#include "Texture.h"
#include "GameSystem.h"
#include "Gimmick.h"
#include "Time.h"
#include "Player.h"
#include "Camera/Camera.h"
#include "Music.h"
#include "Render2D.h"
#include "AudioSystem/AudioSource.h"
#include "editor/Editor.h"
#include "Scene/SceneState.h"
#include "SceneEditor.h"

#include <windows.h>
#include <shobjidl.h>
#include <string>


//シーン遷移分け用
#include "Title.h"
#include "SelectMode.h"
#include "SceneOption.h"
#include "SceneSelectMusic.h"
#include "Scenemaingame.h"
#include "SceneResult.h"
#include "SceneEditor.h"

#include "mglib/note_data.h"



class UI;


class SceneTest
{
public:

	SceneState scene;

	float judgelizeZ;
	SceneTest();
	~SceneTest();


	std::string OpenChartFile();

	bool Initialize();
	void Update();
	void SceneTestRelease();
	void Render();
private:
	
	Shader shader;
	Player pl;
	GameTime gameTime;
	
	Stage stage;
	Gimmick gimmick;
	
	
	UI ui;
	
	NoteData notedata;

	//オプションからタイトルへヒント画像
	Render2D OptionToTitleHint;

	Render2D KeyConfigHint;

	//シーン遷移
	Title title;
	SelectMode selectmode;
	SceneSelectMusic selectmusic;
	SceneEditor sceneeditor;
	SceneOption option;
	Scenemaingame maingame;
	SceneResult result;
	

	Editor editor;

	Editor::EditorData data;
	

	AudioSource music;

	void DebugGUI();
 };

