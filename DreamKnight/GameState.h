#pragma once
#include "EngineState.h"
#include <vector>
#include "Object.h"
class ObjectManager;

class GameState : public EngineState
{
	//ObjectManager * m_ObjManager;
	unsigned int updatecnt = 0;
	Object* player= nullptr;
	std::vector<Object*> allems;
	void Load();
public:
	struct PassedData
	{
		char ip[22];
	};

	PassedData* m_Data;

	GameState(ObjectManager * objMan, PassedData* passedData);
	GameState(ObjectManager * objMan);
	void Update(ObjectManager* om, Renderer * rn, float deltaTime);
	void StartLoad() override;
	~GameState();
};

