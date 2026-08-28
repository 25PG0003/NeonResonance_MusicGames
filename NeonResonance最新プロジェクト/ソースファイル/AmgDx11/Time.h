#pragma once
#include <chrono>

//Gameの時間を計測するクラス

class GameTime
{
public:
	//Editor用
	static void Initialize();
	
	


	//メインゲーム用
	virtual ~GameTime() = default;
	virtual void Update();
	virtual void Start();
	virtual void End();
	virtual void Reset();
	
	float GetElapsedTime();
	float GetDeltaTime();


	void Stop();
	void SetTimeScale(float scale);
	bool IsRunning();
protected:
	float elapsedTime = 0.0f;
	float timeScale = 1.0f;
	float deltaTime = 0.0f;
	bool isRunning = false;
	bool isPaused = false;
	float pauseTime = 0.0f;
};