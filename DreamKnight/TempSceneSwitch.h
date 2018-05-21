#pragma once
#include "Component.h"
#include "DKEngine.h"
#include "GameState.h"
class TempSceneSwitch :
	public Component
{
public:
	TempSceneSwitch();
	~TempSceneSwitch();

	void OnUpdate(float deltaTime)
	{
		//DKEngine::GetInstance()->ChangeState(new GameState(m_GameObject->m_ObjManager));
	}
};

