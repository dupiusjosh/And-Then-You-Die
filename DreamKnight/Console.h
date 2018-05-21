#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <stdio.h>

static FILE* inputStream;
static FILE* outputStream;
static FILE* errorStream;

class Console
{
public:
	static void SetLogFile(char* file);
	static void OpenConsole();
	static void CloseConsole();
};