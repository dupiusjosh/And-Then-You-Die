#include "DevConsole.h"
#include "InputManager.h"
#include "ObjectManager.h"
#include "Renderer.h"


void DevConsole::addChar(char c)
{
	m_Command[pos] = c;
	m_Command[pos + 1] = 0;
	pos++;
}

void DevConsole::Update()
{
	auto input = InputManager::GetInstance();
	if (m_Recording)
	{
		if (pos < 511)
		{
			for (char i = 'A'; i < 'Z'; ++i)
				if (input->IsKeyDown(i))
					addChar(i + 32);
			for (char i = '0'; i < '9'; ++i)
				if (input->IsKeyDown(i))
					addChar(i);
			if (input->IsKeyDown(32))
				addChar(32);
		}
	}

	if (input->IsKeyDown(192))
		m_Recording = true;
	if (input->IsKeyDown('\r'))
		RunCommand();
	if (input->IsKeyDown('\b') && pos > 0)
	{
		m_Command[--pos] = 0;

	}
}

char * DevConsole::getCommand()
{
	return m_Command;
}

void DevConsole::RunCommand()
{
	m_Recording = false;
	Light light;
	if (sscanf_s(m_Command, "\\spawn_point_light %f %f %f %f %f %f",
		&light.m_Position.x,
		&light.m_Position.y,
		&light.m_Position.z,
		&light.m_Color.x,
		&light.m_Color.y,
		&light.m_Color.z) == 6)
	{
		light.m_Position.w = 2.0f;
		Renderer::GetInstance()->AddLight(light);
	}
}
