#include "SceneState.h"

void SceneState::Set(SCENE_GAME scene)
{
    state_main = scene;
}

SCENE_GAME SceneState::Get() const
{
    return state_main;
}

void SceneState::SetNextScene(SCENE_GAME scene)
{
    nextScene = scene;
}

SCENE_GAME SceneState::GetNextScene() const
{
    return nextScene;
}