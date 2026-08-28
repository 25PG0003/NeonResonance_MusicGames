// WinMain.cpp
#include <Windows.h>
#include <tchar.h>
#include "DxSystem.h"
#include "Input/Input.h"
#include "DXShader.h"
#include "RenderState.h"
#include "GameSystem.h"
#include "AudioSystem/AudioSystem.h"

#include "SceneTest.h"
#include "config/config.h"

#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")

#if USE_IMGUI
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#endif



SceneTest sceneTest;
#pragma comment(lib, "winmm.lib")
float DeltaTime = 0; // 経過時間
float RealDeltaTime = 0; // 真の経過時間


#if USE_IMGUI
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
#endif

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#if USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;
#endif

	switch (message) {
	case WM_CLOSE:
		PostMessage(hWnd, WM_DESTROY, 0, 0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return(DefWindowProc(hWnd, message, wParam, lParam));
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	// ウィンドウ生成
	Config::Load();
	TCHAR szWindowClass[] = TEXT("AmgDx11");
	WNDCLASS wcex;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	RegisterClass(&wcex);

	// ウィンドウサイズ補正
	DWORD style = WS_OVERLAPPEDWINDOW;
	bool IsFullScreen = Config::fullscreen;

	if (IsFullScreen)
	{
		style = WS_POPUP;
		nCmdShow = SW_MAXIMIZE;
	}

	RECT WindowSize = { 0,0,1920,1080 };
	int width = 1920;
	int height = 1080;

	if (IsFullScreen)
	{
		width = GetSystemMetrics(SM_CXSCREEN);
		height = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		AdjustWindowRect(&WindowSize, style, FALSE);
		width = WindowSize.right - WindowSize.left;
		height = WindowSize.bottom - WindowSize.top;
	}

	HWND hWnd;
	hWnd = CreateWindow(szWindowClass,
		TEXT("AmgDx11"),
		style,
		0, 0, width, height,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hWnd, nCmdShow);

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	// デバイス初期化
	if (!DxSystem::Initialize(hWnd, 1920, 1080))
	{
		return 0;
	}

#if USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.Fonts->AddFontFromFileTTF(
		"Assets/font/meiryo.ttc",
		20.0f,
		nullptr,
		io.Fonts->GetGlyphRangesJapanese()
	);


	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(DxSystem::Device, DxSystem::DeviceContext);
#endif

	Input::Initialize();
	Shader::InitializeSystem();
	RenderState::Initialize();
	AudioSystem::Initialize();

	// シーン初期化
	sceneTest.Initialize();


	//メインループ
	MSG hMsg = { 0 };
	DWORD timeOld = timeGetTime();
	while (hMsg.message != WM_QUIT) {
		if (PeekMessage(&hMsg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&hMsg);
			DispatchMessage(&hMsg);
		}
		else
		{
			DeltaTime = (timeGetTime() - timeOld) * 0.001f;
			RealDeltaTime = DeltaTime;
			if (DeltaTime > 1.0f / 15) DeltaTime = 1.0f / 15;

			timeOld = timeGetTime();

#if USE_IMGUI
			// imgui update
			ImGuiIO& io = ImGui::GetIO();
			io.DeltaTime = DeltaTime;
			io.DisplaySize.x = (float)DxSystem::ScreenWidth;
			io.DisplaySize.y = (float)DxSystem::ScreenHeight;
			io.MouseDown[0] = GetKeyState(VK_LBUTTON) & 0x8000;
			io.MouseDown[1] = GetKeyState(VK_RBUTTON) & 0x8000;
			ImGui_ImplWin32_NewFrame();
			ImGui_ImplDX11_NewFrame();
			ImGui::NewFrame();
#endif

			Input::Update();

			// 更新・描画
			sceneTest.Update();
			sceneTest.Render();

#if USE_IMGUI
			// Render dear imgui into screen
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

			// 画面更新
			DxSystem::Flip();
		}
	}

	Shader::ReleaseSystem();
	RenderState::Release();
	sceneTest.SceneTestRelease();
	AudioSystem::Release();
	DxSystem::Release();
	CoUninitialize();

#if USE_IMGUI
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif



	return 0;
}
