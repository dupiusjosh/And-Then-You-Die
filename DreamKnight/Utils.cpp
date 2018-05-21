#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "Utils.h"
#include <stdarg.h>
#include <Windows.h>
#include <iostream>

void Utils::ReadFile(const char* filename, char*& Data, long& size)
{
	FILE* file;
	fopen_s(&file, filename, "rb");

	if (!file) {
		size = 0;
		Data = nullptr;
		printf("ERR: failed to open file %s\n", filename);
		return;
	}

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);

	Data = (char*)malloc(size);
	if (!Data)
	{
		fclose(file);
		size = 0;
		Data = nullptr;
		printf("ERR: failed initalize buffer for %s\n", filename);
		return;
	}

	if (!fread(Data, size, 1, file))
	{
		fclose(file);
		free(Data);
		size = 0;
		Data = nullptr;
		printf("ERR: failed to read the file %s\n", filename);
		return;
	}
	fclose(file);
}

int Utils::vk2ascii(DWORD vk, unsigned short result[3])
{
	static HKL layout = GetKeyboardLayout(0);
	static unsigned char State[256];

	if (GetKeyboardState(State) == FALSE)
		return 0;

	UINT scancode = MapVirtualKeyExA(vk, MAPVK_VK_TO_VSC, layout);
	return ToAsciiEx(vk, scancode, State, result, 0, layout);
}

void Utils::PrintError(char * str, ...)
{
	va_list ap;
	va_start(ap, str);
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
	vprintf_s(str, ap);
	SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	va_end(ap);
}

void Utils::PrintWarning(char * str, ...)
{
	va_list ap;
	va_start(ap, str);
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	vprintf_s(str, ap);
	SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	va_end(ap);
}

void Utils::PrintWithColor(uint16_t color, char * str, ...)
{
	va_list ap;
	va_start(ap, str);
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, color);
	vprintf_s(str, ap);
	SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	va_end(ap);
}

bool Utils::isInf(float x)
{
	const float inf = INFINITY;
	return fabs(x) == inf;
}