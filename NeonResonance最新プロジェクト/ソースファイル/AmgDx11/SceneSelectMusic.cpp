#include "SceneSelectMusic.h"
#include "RenderState.h"
#include "Input/Input.h"
#include "imgui/imgui.h"
#include "GameSystem.h"
#include "Gimmick.h"

#include <fstream>
#include <sstream>
#include <windows.h>
#include <filesystem>
#include <vector>
#include <string>
#include "editor/Editor.h"




std::filesystem::path root =
    std::filesystem::current_path() / ".." / "..";

//セレクトミュージックシーン初期化＆読み込み
void SceneSelectMusic::Initialize(SceneState& scene)
{
	ui.Initialize();

	musicselecthint.Initialize("Assets/SelectMusic/musicselecthint.png");
	musicselecthint.SetSize(400, 200);
	musicselecthint.SetPositon(Vector3(20, 10, 0));

	currentjacket.Initialize("Assets/SelectMusic/currentjacket.png");
	currentjacket.SetSize(200, 200);
	currentjacket.SetPositon(Vector3(
		800,
		HEIGHT / 2,
		0
	));
	for (int i = 0; i < 3; i++)
	{
		Jacketholder[i].Initialize("Assets/SelectMusic/jacketholder.png");
		Jacketholder[i].SetSize(200, 200);
		Jacketholder[i].SetPositon(Vector3(300 + 500 * i, HEIGHT / 2, 0));
	}
	
	//譜面読み込み
	LoadChartList();
}

//譜面があるか？ノーツはあるか？
bool SceneSelectMusic::HasChart(int musicIndex, int difficulty) const
{
	if (musicIndex < 0 || musicIndex >= musicList.size())
		return false;

	if (difficulty < 0 || difficulty > 2)
		return false;

	const std::string& path = musicList[musicIndex].chartPath[difficulty];

	if (path.empty())
		return false;

	std::ifstream ifs(path);
	if (!ifs.is_open())
		return false;

	std::string line;
	while (std::getline(ifs, line))
	{
		if (line.empty())
			continue;

		// ヘッダーは飛ばす
		if (line[0] == '#')
			continue;

		// ノーツが1行でもあれば譜面あり
		return true;
	}

	return false;
}


bool SceneSelectMusic::HasAnyChart(int musicIndex) const
{
	return HasChart(musicIndex, 0) ||
		HasChart(musicIndex, 1) ||
		HasChart(musicIndex, 2);
}


void SceneSelectMusic::Update(SceneState& scene,Gimmick& gimmick,AudioSource& music,Scenemaingame& maingame, Editor& editor, Editor::EditorData& data)
{
	if (Input::IsKeyTriggerd(VK_ESCAPE))
	{
		music.Stop("preview");

		step = SelectSong;
		currentDifficulty = 0;

		scene.Set(SELECTMODE);
	}

	if (step == SelectSong)
	{
		if (Input::IsKeyTriggerd(VK_LEFT))  currentmusic--;
		if (Input::IsKeyTriggerd(VK_RIGHT)) currentmusic++;

		if (currentmusic < 0)
			currentmusic = 0;

		if (!musicList.empty() && currentmusic >= static_cast<int>(musicList.size()))
			currentmusic = static_cast<int>(musicList.size()) - 1;

		if (!musicList.empty() && currentmusic != lastPreviewMusic)
		{
			lastPreviewMusic = currentmusic;

			OutputDebugStringA("Preview musicPath = ");
			OutputDebugStringA(musicList[currentmusic].musicPath.c_str());
			OutputDebugStringA("\n");

			OutputDebugStringA("Preview start\n");
			music.Stop("preview");

			bool loadOK = true;

			// 曲が変わった時だけLoadする
			if (loadedPreviewMusic != currentmusic)
			{
				loadOK = music.Load(
					"preview",
					musicList[currentmusic].musicPath.c_str(),
					true
				);

				if (loadOK)
					loadedPreviewMusic = currentmusic;
			}

			if (!loadOK)
			{
				MessageBoxA(
					nullptr,
					musicList[currentmusic].musicPath.c_str(),
					"Preview Music Load Failed",
					MB_OK
				);
				return;
			}

			music.Seek("preview", musicList[currentmusic].previewTime);
			music.Play("preview");
		}

		if (Input::IsKeyTriggerd(VK_RETURN))
		{
			if (scene.GetNextScene() == EditorMode)
			{
				selectedChart = musicList[currentmusic].chartPath[0];

				gimmick.LoadCSV(selectedChart);
				music.Stop("preview");

				maingame.LoadMusicFromChart(gimmick, music);

				data.musicPath = gimmick.GetMusicPath();
				data.bpm = gimmick.GetBPM();

				editor.LoadFromGimmick(data);

				scene.Set(EditorMode);
				return;
			}

			// 通常プレイ
			if (!HasAnyChart(currentmusic))
			{
				return;
			}

			step = SelectDifficulty;
			currentDifficulty = 0;
		}
	}
	else if (step == SelectDifficulty)
	{
		if (Input::IsKeyTriggerd(VK_UP))   currentDifficulty--;
		if (Input::IsKeyTriggerd(VK_DOWN)) currentDifficulty++;

		if (currentDifficulty < 0) currentDifficulty = 0;
		if (currentDifficulty > 2) currentDifficulty = 2;


		if (Input::IsKeyTriggerd(VK_BACK))
		{
			// lastPreviewMusic = -1; ←消す
			step = SelectSong;
		}

		

		if (Input::IsKeyTriggerd(VK_RETURN))
		{
			if (musicList.empty())
				return;

			if (!HasChart(currentmusic, currentDifficulty))
			{
				return;
			}

			selectedChart = musicList[currentmusic].chartPath[currentDifficulty];

			if (!selectedChart.empty())
			{
				gimmick.LoadCSV(selectedChart);
				music.Stop("preview");

				step = SelectSong;
				currentDifficulty = 0;
				lastPreviewMusic = -1;

				maingame.LoadMusicFromChart(gimmick, music);

				editor.LoadMusic(gimmick.GetMusicPath());

				// Editor用にも初期化
				data.musicPath = musicList[currentmusic].musicPath;
				data.bpm = musicList[currentmusic].bpm;

				editor.LoadFromGimmick(data);
				scene.Set(EditorMode);

				editor.LoadFromGimmick(data);

				scene.Set(scene.GetNextScene());
				return;
			}
		}
	}
}

void SceneSelectMusic::Release(SceneState& scene)
{
	for (auto* img : jacketImages)
	{
		delete img;
	}

	jacketImages.clear();
}

void SceneSelectMusic::Render(SceneState& scene)
{
	if (scene.Get() == SELECTMUSIC)
	{


		if (step == SelectDifficulty)
		{
			float x = 300 + 500 * currentmusic;
			float y = HEIGHT / 2 - 180;
			ImVec2 windowSize(80, 80);
			ImGui::SetNextWindowPos(ImVec2(WIDTH / 2 -165, HEIGHT / 2 -250));

			ImGui::SetNextWindowSize(ImVec2(235, 220));
			ImGui::Begin("Difficulty", nullptr,
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoScrollbar|
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove);
			ImGui::SetWindowFontScale(3.0f);

			ImGui::Text(currentDifficulty == 0 ? "> EASY" : "  EASY");
			ImGui::Text(currentDifficulty == 1 ? "> NORMAL" : "  NORMAL");
			ImGui::Text(currentDifficulty == 2 ? "> HARD" : "  HARD");

			ImGui::End();
		}

		ui.titlepix.Render();
		musicselecthint.Render();


		currentjacket.SetPositon(Vector3(800, HEIGHT / 2, 0));
		currentjacket.Render();
		

		for (int i = -1; i <= 1; i++)
		{

			int index = currentmusic + i;


			if (index < 0 || index >= static_cast<int>(musicList.size()))
				continue;

			if (index >= static_cast<int>(jacketImages.size()))
				continue;

			float x = 450 + (i + 1) * 350;
			float y = HEIGHT / 2 + 10;

			jacketImages[index]->SetPositon(Vector3(x + 10, y, 0));
			jacketImages[index]->Render();
		}
	}
}

void SceneSelectMusic::LoadChartList()
{


	std::ifstream ifs("Assets/chart/chart.csv");
	if (!ifs.is_open())
	{
		MessageBoxA(nullptr, "open failed", "chart.csv", MB_OK);
		return;
	}



	std::string line;
	std::getline(ifs, line); // header skip

	while (std::getline(ifs, line))
	{
		

		if (line.empty()) continue;
		if (!line.empty() && line.back() == '\r') line.pop_back();

		std::stringstream ss(line);

		MusicData data;
		std::string previewStr;

		std::getline(ss, data.name, ',');
		std::getline(ss, data.jacketPath, ',');
		std::getline(ss, data.musicPath, ',');
		std::getline(ss, previewStr, ',');

		std::getline(ss, data.chartPath[0], ',');
		std::getline(ss, data.chartPath[1], ',');
		std::getline(ss, data.chartPath[2], ',');

		// ★追加（ここでBPM持たせる）
		std::string bpmStr;
		if (std::getline(ss, bpmStr, ','))
		{
			try {
				data.bpm = std::stof(bpmStr);
			}
			catch (...) {
				data.bpm = 120.0f;
			}
		}
		else
		{
			data.bpm = 120.0f;
		}

		try
		{
			data.previewTime = std::stof(previewStr);
		}
		catch (...)
		{
			data.previewTime = 0.0f;
		}

		if (data.name.empty() || data.jacketPath.empty())
			continue;

		musicList.push_back(data);

		Render2D* jacket = new Render2D();
		jacket->Initialize(data.jacketPath.c_str());
		jacket->SetSize(180, 180);

		jacketImages.push_back(jacket);
	}
}