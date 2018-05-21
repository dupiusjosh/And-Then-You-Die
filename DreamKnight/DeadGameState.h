#pragma once
#include "EngineState.h"
class DeadGameState : public EngineState
{
	void Load() override;
public:
	DeadGameState(ObjectManager * objMan);
	~DeadGameState();
};

