#include "SceneResult.h"
#include "Gimmick.h"
#include "Input/Input.h"
#include "Scenemaingame.h"
#include "mgLib/note_data.h"

void SceneResult::Initialize(SceneState& scene)
{
    Result_Screen.Initialize("Assets/Result/resultBG.png");
	Result_Screen.SetSize(1920, 1080);
    Result_Screen.SetPositon(Vector3(0, 0, 0));
}

void SceneResult::Update(SceneState& scene, Scenemaingame& maingame, Editor& editor, GameTime& gameTime, Gimmick& gimmick, AudioSource& music)
{
    if (Input::IsKeyTriggerd(VK_RETURN))
    {
        scene.Set(SELECTMODE);
        maingame.ResetGame(editor, gameTime, gimmick,music);
        return;
    }
}

void SceneResult::Release(SceneState& scene)
{
   
}

void SceneResult::Render(SceneState& scene,Gimmick& gimmick)
{
    Result_Screen.Render();
    gimmick.ComboUpdate(600, 615, gimmick.notedata.GetPerfectCount());
    gimmick.ComboUpdate(600, 705, gimmick.notedata.GetGoodCount());
    gimmick.ComboUpdate(600, 800, gimmick.notedata.GetMissCount());
    gimmick.ComboUpdate(1300, 640, gimmick.notedata.GetComboCount());

    gimmick.ComboUpdate(525, 290, gimmick.notedata.GetScore());
    gimmick.ComboUpdate(1240, 800, gimmick.notedata.GetFastCount());
    gimmick.ComboUpdate(980, 800, gimmick.notedata.GetLateCount());
}
