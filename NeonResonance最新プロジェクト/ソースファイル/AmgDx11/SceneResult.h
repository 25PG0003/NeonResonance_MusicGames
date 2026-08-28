#pragma once
#include "Scene/SceneState.h"
#include "Render2D.h"
#include "Gimmick.h"
#include "Input/Input.h"
#include "editor/Editor.h"
#include "Time.h"
#include "Scenemaingame.h"
#include "mgLib/note_data.h"

class SceneResult
{
public:
	void Initialize(SceneState& scene);
	void Update(SceneState& scene, Scenemaingame& maingame, Editor& editor, GameTime& gameTime, Gimmick& gimmick, AudioSource& music);
	void Release(SceneState& scene);
	void Render(SceneState& scene,Gimmick& gimmick);
private:
	//リザルト画面読み込み
	Render2D Result_Screen;
};

