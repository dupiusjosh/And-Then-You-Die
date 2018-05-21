#pragma once
#include "EngineState.h"
#include <vector>
#include <string>
class GuiText;
class CreditState :
	public EngineState
{
	std::vector<std::string> Lines;
	std::vector<GuiText*> textboxes;
	float timeSinceSwitch=100.0f;
	float timeTilSwitch=5.0f;
	int curIndx = 0;

public:
	CreditState(ObjectManager * objMan);
	virtual ~CreditState() {}

	void Load();
	void StartLoad() override;
	void Update(ObjectManager* om, Renderer * rn, float deltaTime);

};

