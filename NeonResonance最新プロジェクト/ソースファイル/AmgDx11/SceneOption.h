#pragma once

#include "Scene/SceneState.h"

#include "imgui/imgui.h"
#include "CommonUI.h"
#include "GameSystem.h"

#include "Stage.h"
#include "Gimmick.h"




class SceneOption
{
public:
	void Initialize(SceneState& scene);
	void Update(SceneState& scene,Gimmick& gimmick);
	void Render(SceneState& scene,UI& ui);
private:
	Stage PreviewStage;
	Gimmick PreviewGimmick;

	float previewTime = 0.0f;
};

