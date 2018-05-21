#pragma once
#include "Component.h"
#include "Utils.h"
#include <string>
#include <Windows.h>

class ScreamingComponent : public Component
{
	std::string m_Name;
public:
	ScreamingComponent(std::string name)
	{
		m_Name = name;
	}

	void OnUpdate(float deltaTime)
	{
		Utils::PrintWithColor(FOREGROUND_RED | FOREGROUND_INTENSITY, "I'M %s AND I'M UPDATING!!!\n", m_Name.c_str());
	}
};