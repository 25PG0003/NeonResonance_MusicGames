#include "Render2D.h"
#include "Scene/SceneState.h"
#include "CommonUI.h"
#include "imgui/imgui.h"


class Title
{
public:
	Title() {};
	~Title() {};

	bool Initialize(SceneState& scene);
	void Release();
	void Update(SceneState& scene);
	void Render(SceneState& scene);

	void UIAnim();
private:

	//タイトル画面表示
		//タイトル
	
	Render2D titleSprite;
	Render2D SceneButton;
	
	UI ui;


	//タイトル用アニメーション変数
	float titleAnimTime = 0.0f;
	
	
	
};