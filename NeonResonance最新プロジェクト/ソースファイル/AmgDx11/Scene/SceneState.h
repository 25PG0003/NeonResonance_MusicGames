#pragma once


class SceneTest;

enum SCENE_GAME
{
	TITLE,
	Difficult,
	MainGame,
	Result,
	Option,
	Puase,
	Failed,
	EditorMode,
	ConfirmExit,
	SELECTMODE,
	SELECTMUSIC,


	//実装前用
	UnderConstruction,
};


class SceneState
{
public:
	void Set(SCENE_GAME scene);
	SCENE_GAME Get() const;

	void SetNextScene(SCENE_GAME scene);
	SCENE_GAME GetNextScene() const;

private:
	SCENE_GAME state_main = TITLE;
	SCENE_GAME nextScene = MainGame;

};