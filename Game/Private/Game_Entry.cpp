#include "pch.h"
#include "Game_Entry.h"
#include "CTestScene.h"

namespace Game
{
    CScene* Create_TestScene(const Engine::CREATE_SCENE_DESC& Desc)
    {
        return CTestScene::Create(Desc);
    }
}
