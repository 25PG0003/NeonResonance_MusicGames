#pragma once

#include "editor/Editor.h"
#include "Scene/SceneState.h"
#include "Time.h"

class SceneEditor
{

public:
	SceneEditor() {};
	~SceneEditor() {};

	void Initialize(SceneState& scene, Editor& editor);
	void Render(SceneState& scene, Editor& editor, Editor::EditorData& data);
	void EditorUpdate(SceneState& scene, GameTime& gameTime, Editor& editor);
	void UnderConstructionUpdate(SceneState& scene, Editor& editor);
	


private:
	
	//Editor用シーン管理変数
	bool showSuccess = false;
	bool openSuccessPopup = false;
	bool showSaveSuccessPopup = false;
	bool musicSelected = false;


};

