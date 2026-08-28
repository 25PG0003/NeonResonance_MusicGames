// Input.cpp
#include <Windows.h>
#include <math.h>
#include "Input.h"

#pragma comment(lib,"xinput.lib")

_XINPUT_STATE XInput_State;
SHORT val;

Input::States Input::states[MAX_CONTROLLERS];
int Input::BUTTON_MAP[16];
int Input::KEY_MAP[16];

namespace
{
	bool IsAsyncDown(int vk)
	{
		return (GetAsyncKeyState(vk) & 0x8000) != 0;
	}

	bool IsDebouncedDown(int vk, DWORD debounceMs)
	{
		struct DebounceState
		{
			bool initialized = false;
			bool stableDown = false;
			bool lastRawDown = false;
			ULONGLONG lastChangeTime = 0;
		};

		static DebounceState state[256];
		const int idx = vk & 0xFF;
		const ULONGLONG now = GetTickCount64();
		const bool rawDown = IsAsyncDown(vk);

		DebounceState& s = state[idx];

		if (!s.initialized)
		{
			s.initialized = true;
			s.stableDown = rawDown;
			s.lastRawDown = rawDown;
			s.lastChangeTime = now;
			return s.stableDown;
		}

		if (rawDown != s.lastRawDown)
		{
			s.lastRawDown = rawDown;
			s.lastChangeTime = now;
		}

		if (rawDown != s.stableDown && (now - s.lastChangeTime) >= debounceMs)
		{
			s.stableDown = rawDown;
		}

		return s.stableDown;
	}

	void UpdateDigitalState(int& state, bool isDown)
	{
		int old = state & 0x01;
		state = isDown ? 1 : 0;

		if ((state & 0x01) != old)
		{
			state |= 0x02;
		}
	}

	void ApplyStickDeadZone(float x, float y, float deadZone, float& outX, float& outY)
	{
		float d = sqrtf(x * x + y * y);

		if (d < deadZone)
		{
			outX = 0.0f;
			outY = 0.0f;
			return;
		}

		if (d > 1.0f)
		{
			x /= d;
			y /= d;
			d = 1.0f;
		}

		float scale = (d - deadZone) / (1.0f - deadZone);
		outX = x / d * scale;
		outY = y / d * scale;
	}

	float ApplyTriggerDeadZone(float value, float deadZone)
	{
		if (value <= deadZone)
		{
			return 0.0f;
		}

		return (value - deadZone) / (1.0f - deadZone);
	}
}

void Input::Initialize()
{
	int init_map[16] = {
		XINPUT_GAMEPAD_DPAD_UP,
		XINPUT_GAMEPAD_DPAD_DOWN,
		XINPUT_GAMEPAD_DPAD_LEFT,
		XINPUT_GAMEPAD_DPAD_RIGHT,
		XINPUT_GAMEPAD_START,
		XINPUT_GAMEPAD_BACK,
		XINPUT_GAMEPAD_LEFT_THUMB,
		XINPUT_GAMEPAD_RIGHT_THUMB,
		XINPUT_GAMEPAD_LEFT_SHOULDER,
		XINPUT_GAMEPAD_RIGHT_SHOULDER,
		XINPUT_GAMEPAD_A,
		XINPUT_GAMEPAD_B,
		XINPUT_GAMEPAD_X,
		XINPUT_GAMEPAD_Y,
	};

	int init_keymap[16] = {
		VK_UP,
		VK_DOWN,
		VK_LEFT,
		VK_RIGHT,
		VK_RETURN,
		VK_BACK,
		VK_SHIFT,
		VK_TAB,
		VK_CONTROL,
		VK_SPACE,
		'Z',
		'X',
		'C',
		'V',
	};

	for (int i = 0; i < 16; i++)
	{
		BUTTON_MAP[i] = init_map[i];
		KEY_MAP[i] = init_keymap[i];
	}

	// 追加: 初期化時に全状態をクリア
	for (int i = 0; i < MAX_CONTROLLERS; i++)
	{
		ZeroMemory(&states[i], sizeof(States));
	}
}

void Input::Update()
{
	static POINT prevPos = {};
	static bool first = true;

	auto ResetControllerState = [](int index)
	{
		states[index].AxisLX = 0.0f;
		states[index].AxisLY = 0.0f;
		states[index].AxisRX = 0.0f;
		states[index].AxisRY = 0.0f;
		states[index].LeftTrigger = 0.0f;
		states[index].RightTrigger = 0.0f;

		for (int b = 0; b < 16; b++)
		{
			states[index].Buttons[b] = RELEASED;
		}
	};

	POINT pos = {};
	HWND hwnd = GetActiveWindow();

	if (hwnd != nullptr)
	{
		GetCursorPos(&pos);
		ScreenToClient(hwnd, &pos);

		states[0].MouseX = pos.x;
		states[0].MouseY = pos.y;

		if (first)
		{
			states[0].MouseDeltaX = 0;
			states[0].MouseDeltaY = 0;
			first = false;
		}
		else
		{
			states[0].MouseDeltaX = pos.x - prevPos.x;
			states[0].MouseDeltaY = pos.y - prevPos.y;
		}

		prevPos = pos;
	}
	else
	{
		states[0].MouseDeltaX = 0;
		states[0].MouseDeltaY = 0;
	}

	// マウスボタン
	int mouseKeys[3] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON };

	for (int i = 0; i < 3; i++)
	{
		bool isDown = IsDebouncedDown(mouseKeys[i], MOUSE_DEBOUNCE_MS);
		UpdateDigitalState(states[0].MouseButtons[i], isDown);
	}

	for (DWORD i = 0; i < MAX_CONTROLLERS; i++)
	{
		_XINPUT_STATE state;
		ZeroMemory(&state, sizeof(_XINPUT_STATE));
		DWORD dwResult = XInputGetState(i, &state);

		if (dwResult == ERROR_SUCCESS)
		{
			// 左スティック
			{
				float x = state.Gamepad.sThumbLX / 32767.0f;
				float y = state.Gamepad.sThumbLY / 32767.0f;

				// キーボード補助
				if (IsAsyncDown('W')) y += 1.0f;
				if (IsAsyncDown('A')) x -= 1.0f;
				if (IsAsyncDown('S')) y -= 1.0f;
				if (IsAsyncDown('D')) x += 1.0f;

				ApplyStickDeadZone(x, y, STICK_DEAD_ZONE, states[i].AxisLX, states[i].AxisLY);
			}

			// 右スティック
			{
				float x = state.Gamepad.sThumbRX / 32767.0f;
				float y = state.Gamepad.sThumbRY / 32767.0f;

				if (IsAsyncDown(VK_NUMPAD8)) y += 1.0f;
				if (IsAsyncDown(VK_NUMPAD4)) x -= 1.0f;
				if (IsAsyncDown(VK_NUMPAD2)) y -= 1.0f;
				if (IsAsyncDown(VK_NUMPAD6)) x += 1.0f;

				ApplyStickDeadZone(x, y, STICK_DEAD_ZONE, states[i].AxisRX, states[i].AxisRY);
			}

			// トリガー
			{
				states[i].LeftTrigger = ApplyTriggerDeadZone(state.Gamepad.bLeftTrigger / 255.0f, TRIGGER_DEAD_ZONE);
				if (IsAsyncDown('Q')) states[i].LeftTrigger = 1.0f;

				states[i].RightTrigger = ApplyTriggerDeadZone(state.Gamepad.bRightTrigger / 255.0f, TRIGGER_DEAD_ZONE);
				if (IsAsyncDown('E')) states[i].RightTrigger = 1.0f;
			}

			// ボタン
			for (int b = 0; b < 16; b++)
			{
				int old = states[i].Buttons[b] & 0x01;

				bool isDown = ((state.Gamepad.wButtons & BUTTON_MAP[b]) != 0) ||
					(IsDebouncedDown(KEY_MAP[b], KEY_DEBOUNCE_MS));

				states[i].Buttons[b] = isDown ? 1 : 0;

				if (states[i].Buttons[b] != old)
				{
					states[i].Buttons[b] |= 0x02;
				}
			}
		}
		else
		{
			// 追加: 切断時は状態をリセット
			ResetControllerState((int)i);

			if (i == 0)
			{
				// キーボード代替入力は維持
				// 左スティック
				{
					float x = 0.0f;
					float y = 0.0f;

					if (IsAsyncDown('W')) y += 1.0f;
					if (IsAsyncDown('A')) x -= 1.0f;
					if (IsAsyncDown('S')) y -= 1.0f;
					if (IsAsyncDown('D')) x += 1.0f;

					ApplyStickDeadZone(x, y, STICK_DEAD_ZONE, states[i].AxisLX, states[i].AxisLY);
				}

				// 右スティック
				{
					float x = 0.0f;
					float y = 0.0f;

					if (IsAsyncDown(VK_NUMPAD8)) y += 1.0f;
					if (IsAsyncDown(VK_NUMPAD4)) x -= 1.0f;
					if (IsAsyncDown(VK_NUMPAD2)) y -= 1.0f;
					if (IsAsyncDown(VK_NUMPAD6)) x += 1.0f;

					ApplyStickDeadZone(x, y, STICK_DEAD_ZONE, states[i].AxisRX, states[i].AxisRY);
				}

				// トリガー
				{
					states[i].LeftTrigger = IsAsyncDown('Q') ? 1.0f : 0.0f;
					states[i].RightTrigger = IsAsyncDown('E') ? 1.0f : 0.0f;
				}

				// ボタン
				for (int b = 0; b < 16; b++)
				{
					int old = states[i].Buttons[b] & 0x01;
					bool isDown = IsDebouncedDown(KEY_MAP[b], KEY_DEBOUNCE_MS);

					states[i].Buttons[b] = isDown ? 1 : 0;

					if (states[i].Buttons[b] != old)
					{
						states[i].Buttons[b] |= 0x02;
					}
				}
			}
		}
	}
}

float Input::GetAxisX(int index)
{
	return states[index].AxisLX;
}

float Input::GetAxisY(int index)
{
	return states[index].AxisLY;
}

float Input::GetAxisRX(int index)
{
	return states[index].AxisRX;
}

float Input::GetAxisRY(int index)
{
	return states[index].AxisRY;
}

int Input::GetButton(int index, BUTTON button)
{
	return states[index].Buttons[button];
}

void Input::SetVibration(int index, float left, float right)
{
	if (left < 0) left = 0;
	if (left > 1) left = 1;
	if (right < 0) right = 0;
	if (right > 1) right = 1;

	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = (WORD)(left * 65535);
	vibration.wRightMotorSpeed = (WORD)(right * 65535);
	XInputSetState(index, &vibration);
}

bool Input::IsKeyTriggerd(int key)
{
	static short prev[256] = {};

	short now = GetAsyncKeyState(key);
	bool triggered = (now & 0x8000) && !(prev[key] & 0x8000);

	prev[key] = now;
	return triggered;
}

int Input::GetMouseX(int index)
{
	return states[index].MouseX;
}

int Input::GetMouseY(int index)
{
	return states[index].MouseY;
}

int Input::GetMouseDeltaX(int index)
{
	return states[index].MouseDeltaX;
}

int Input::GetMouseDeltaY(int index)
{
	return states[index].MouseDeltaY;
}

int Input::GetMouseButton(int index, MOUSE_BUTTON button)
{
	return states[index].MouseButtons[button];
}

bool Input::IsKeyDown(int key)
{
	return (GetAsyncKeyState(key) & 0x8000) != 0;
}

int Input::IsMouseTriggerd(int index)
{
	if (index < 0 || index >= 3)
	{
		return false;
	}

	return states[0].MouseButtons[index] == JUST_PRESSED;
}