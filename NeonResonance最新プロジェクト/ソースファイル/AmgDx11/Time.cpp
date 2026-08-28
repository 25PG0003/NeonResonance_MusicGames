#include "Time.h"
#include "Input/Input.h"
#include "SceneTest.h"

void GameTime::Start()
{
	elapsedTime = 0.0f;
	DeltaTime = 0.0f;
	isRunning = true;
	isPaused = false;
}

void GameTime::Stop()
{
	isPaused = true;
}

void GameTime::End()
{
	isRunning = false;
}

void GameTime::Update()
{
	if (!isRunning || isPaused)
		return;

	deltaTime = DeltaTime * timeScale;

	elapsedTime += deltaTime;


}

void GameTime::Initialize()
{

}

void GameTime::Reset()
{
	elapsedTime = 0.0f;
	deltaTime = 0.0f;
	isPaused = false;
	isRunning = false;  
}

float GameTime::GetElapsedTime()
{
	return elapsedTime;
}

float GameTime::GetDeltaTime()
{
	return DeltaTime;
}

void GameTime::SetTimeScale(float scale)
{
	timeScale = scale;
}

bool GameTime::IsRunning()
{
	return isRunning;
}