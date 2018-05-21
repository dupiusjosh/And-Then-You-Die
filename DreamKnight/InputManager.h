#pragma once
#include <vector>
#include <map>
#include "InputType.h"

typedef unsigned char byte;

class InputManager
{
private:
	// reference Gateware\Include\Interface\G_System\GKeyDefines for key numbers
	InputManager(const InputManager&);	
	//static InputManager * m_Instance;
	std::map<unsigned int, uint8_t> keys;
	
	float* m_KeyHoldTime;
	bool* m_KeyOldFrame;
	bool* m_KeyCurrFrame;
	int temp = 0;
	int m_xPos=0, m_yPos = 0, m_OldXPos = 0, m_OldYPos = 0;

public:

	enum class KEYMAP : unsigned int
	{
		FORWARD = 0,
		BACKWARD,
		LEFT,
		RIGHT,
		JUMP,
		INTRACT,
		PRIMARY_ATTACK,
		SECONDARY_ATTACK,
		TERTIARY_ATTACK,
		SUPER_ABILITY,
		MOBILE_ABILITY,
		DEFENSE_ABILITY,
		KEYMAPCOUNT // used to send data.
	};
	InputManager();
	InputManager(std::map<unsigned int, uint8_t> _keys);
	~InputManager();

	static InputManager * GetInstance()
	{
		static InputManager * inst = new InputManager();
		return inst;
	}
	void Update(float delta);
	void SetKeyPressed(uint8_t key);
	void SetKeyUp(uint8_t key);

	void SetKeyPressed(KEYMAP key);
	void SetKeyUp(KEYMAP key);

	bool IsKeyPressed(KEYMAP key);
	bool IsKeyDown(KEYMAP key);
	bool IsKeyUp(KEYMAP key);
	float GetKeyHoldTime(KEYMAP key);

	bool IsKeyPressed(uint8_t key);
	bool IsKeyDown(uint8_t key);
	bool IsKeyUp(uint8_t key);
	float GetKeyHoldTime(uint8_t key);

	//static InputManager * GetInstance() {
	//	return (!m_Instance) ? m_Instance = new InputManager :m_Instance;
	//}
	void SetCurrentMousePos(int xPos,  int yPos);
	void GetCurrentMousePos(int& xPos, int& yPos);
	void GetDeltaMousePos(  int& xPos, int& yPos);
};