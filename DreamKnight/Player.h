#pragma once
#include "Object.h"
#include "InputType.h"
#include "InputManager.h"

struct Player : public Component
{
private:
	InputManager* m_Input;// = m_Input->GetInstance();
public:
	Player(Object * _obj);
	~Player();
	void OnUpdate(float _dTime) override;
};
