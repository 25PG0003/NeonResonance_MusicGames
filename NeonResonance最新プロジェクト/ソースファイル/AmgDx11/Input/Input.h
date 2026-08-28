#pragma once
#include <Windows.h>
#include <XInput.h>

class Input
{
private:
	static const int MAX_CONTROLLERS = 4;

	// ボタン/キーのマップ
	static int BUTTON_MAP[16];
	static int KEY_MAP[16];

	// 入力状態保存用
	struct States {
		float AxisLX;
		float AxisLY;
		float AxisRX;
		float AxisRY;

		float LeftTrigger;
		float RightTrigger;

		int Buttons[16];

		// マウス対応
		int MouseX;
		int MouseY;
		int MouseDeltaX;
		int MouseDeltaY;
		int MouseButtons[3]; // 0:左, 1:右, 2:中央
	};
	static States states[MAX_CONTROLLERS];

public:
	enum BUTTON {
		UP = 0,
		DOWN,
		LEFT,
		RIGHT,
		START,
		BACK,
		LSTICK,
		RSTICK,
		LB,
		RB,
		A,
		B,
		X,
		Y,
	};

	enum MOUSE_BUTTON
	{
		MOUSE_RIGHT,
		MOUSE_LEFT = 0,
		MOUSE_CENTER,
	};

	static const int RELEASED = 0;
	static const int PRESSED = 1;
	static const int JUST_RELEASED = 2;
	static const int JUST_PRESSED = 3;

	// チャタリング対策用
	static constexpr DWORD KEY_DEBOUNCE_MS = 8;
	static constexpr DWORD MOUSE_DEBOUNCE_MS = 4;
	static constexpr float STICK_DEAD_ZONE = 0.20f;
	static constexpr float TRIGGER_DEAD_ZONE = 0.20f;

	static void Initialize();
	static void Update();

	static float GetAxisX(int index = 0);
	static float GetAxisY(int index = 0);
	static float GetAxisRX(int index = 0);
	static float GetAxisRY(int index = 0);
	static int GetButton(int index, BUTTON button);
	static bool IsKeyTriggerd(int key);
	static int GetMouseX(int index = 0);
	static int GetMouseY(int index = 0);
	static int GetMouseDeltaX(int index = 0);
	static int GetMouseDeltaY(int index = 0);
	static int GetMouseButton(int index, MOUSE_BUTTON button);
	static bool IsKeyDown(int key);
	static int IsMouseTriggerd(int index);

	static void SetVibration(int index, float left, float right);
};

