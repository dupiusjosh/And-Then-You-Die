#include "InputManager.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

//InputManager * InputManager::m_Instance = nullptr;

InputManager::InputManager()
{
	keys.insert(std::make_pair((unsigned int)KEYMAP::FORWARD, 'W'));
	keys.insert(std::make_pair((unsigned int)KEYMAP::LEFT, 'A'));
	keys.insert(std::make_pair((unsigned int)KEYMAP::BACKWARD, 'S'));
	keys.insert(std::make_pair((unsigned int)KEYMAP::RIGHT, 'D'));
	keys.insert(std::make_pair((unsigned int)KEYMAP::JUMP, VK_SPACE));
	keys.insert(std::make_pair((unsigned int)KEYMAP::INTRACT, 'E'));
	keys.insert(std::make_pair((unsigned int)KEYMAP::PRIMARY_ATTACK, VK_LBUTTON));
	keys.insert(std::make_pair((unsigned int)KEYMAP::SECONDARY_ATTACK, VK_RBUTTON));
	keys.insert(std::make_pair((unsigned int)KEYMAP::TERTIARY_ATTACK, 'Q'));
	keys.insert(std::make_pair((unsigned int)KEYMAP::SUPER_ABILITY, 'R'));
	keys.insert(std::make_pair((unsigned int)KEYMAP::MOBILE_ABILITY, VK_SHIFT));
	keys.insert(std::make_pair((unsigned int)KEYMAP::DEFENSE_ABILITY, 'F'));

	m_KeyHoldTime = new float[256];
	m_KeyCurrFrame = new bool[256];
	m_KeyOldFrame = new bool[256];

	memset(m_KeyCurrFrame, 0, sizeof(m_KeyCurrFrame[0]) * 256);
	memset(m_KeyOldFrame, 0, sizeof(m_KeyOldFrame[0]) * 256);
	memset(m_KeyHoldTime, 0, sizeof(m_KeyHoldTime[0]) * 256);
}

InputManager::InputManager(std::map<unsigned int, uint8_t> _keys)
{
	keys = _keys;
	m_KeyHoldTime = new float[256];
	m_KeyCurrFrame = new bool[256];
	m_KeyOldFrame = new bool[256];
	memset(m_KeyCurrFrame, 0, sizeof(m_KeyCurrFrame[0]) * 256);
	memset(m_KeyOldFrame, 0, sizeof(m_KeyOldFrame[0]) * 256);
	memset(m_KeyHoldTime, 0, sizeof(m_KeyHoldTime[0]) * 256);
}

InputManager::~InputManager()
{
	delete[] m_KeyOldFrame;
	delete[] m_KeyCurrFrame;
	delete[] m_KeyHoldTime;
}

void InputManager::Update(float delta)
{
	m_OldXPos = m_xPos;
	m_OldYPos = m_yPos;
	memcpy_s(m_KeyOldFrame, 256, m_KeyCurrFrame, 256);
	for (int i = 0; i < 256; ++i)
		if (m_KeyCurrFrame[i] == true)
			m_KeyHoldTime[i] += delta;
}

void InputManager::SetKeyPressed(uint8_t key)
{
	m_KeyCurrFrame[key] = true;
}

void InputManager::SetKeyUp(uint8_t key)
{
	m_KeyCurrFrame[key] = false;
	m_KeyHoldTime[key] = 0.0f;
}

void InputManager::SetKeyPressed(KEYMAP key)
{
	uint8_t actualKey = keys[(unsigned int)key];

	m_KeyCurrFrame[actualKey] = true;
}

void InputManager::SetKeyUp(KEYMAP key)
{
	uint8_t actualKey = keys[(unsigned int)key];
	m_KeyHoldTime[actualKey] = 0.0f;
	m_KeyCurrFrame[actualKey] = false;
}

bool InputManager::IsKeyPressed(KEYMAP key)
{
	uint8_t actualKey = keys[(unsigned int)key];

	return m_KeyCurrFrame[actualKey];
}

bool InputManager::IsKeyDown(KEYMAP key)
{
	uint8_t actualKey = keys[(unsigned int)key];
	return ((m_KeyCurrFrame[actualKey] && !m_KeyOldFrame[actualKey]) ? true : false);
}

bool InputManager::IsKeyUp(KEYMAP key)
{
	uint8_t actualKey = keys[(unsigned int)key];
	return ((!m_KeyCurrFrame[actualKey] && m_KeyOldFrame[actualKey]) ? true : false);
}

float InputManager::GetKeyHoldTime(KEYMAP key)
{
	uint8_t actualKey = keys[(unsigned int)key];
	return m_KeyHoldTime[actualKey];
}

bool InputManager::IsKeyPressed(uint8_t key)
{
	return m_KeyCurrFrame[key];
}

bool InputManager::IsKeyDown(uint8_t key)
{
	return ((m_KeyCurrFrame[key] && !m_KeyOldFrame[key]) ? true : false);

}

bool InputManager::IsKeyUp(uint8_t key)
{
	return ((!m_KeyCurrFrame[key] && m_KeyOldFrame[key]) ? true : false);

}

float InputManager::GetKeyHoldTime(uint8_t key)
{
	return m_KeyHoldTime[key];
}

void InputManager::SetCurrentMousePos(int xPos, int yPos)
{
	m_xPos = xPos;
	m_yPos = yPos;
}

void InputManager::GetCurrentMousePos(int& xPos, int& yPos)
{
	xPos = m_xPos;
	yPos = m_yPos;
}

void InputManager::GetDeltaMousePos(int& xPos, int& yPos)
{
	xPos = m_xPos - m_OldXPos;
	yPos = m_yPos - m_OldYPos;
	//printf("MOUSE DELTA: %i %i %i %i", m_OldXPos, m_OldYPos, m_xPos,m_yPos);
}
