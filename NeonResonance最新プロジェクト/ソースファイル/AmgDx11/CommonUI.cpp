#include "CommonUI.h"
#include "Render2D.h"

bool UI::Initialize()
{
	//タイトル背景画像読み込み
	titlepix.Initialize("Assets/title/title.png");
	titlepix.SetSize(1920, 1080);
	titlepix.SetPositon(Vector3(0, 0, 0));

	return true;
}