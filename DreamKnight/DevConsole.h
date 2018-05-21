#pragma once
#include <string>
class DevConsole
{
	uint16_t pos = 0;
	char m_Command[512] = "Hello World!";
	bool m_Recording = false;
	void addChar(char c);
public:
	void Update();
	char* getCommand();
	void RunCommand();
};