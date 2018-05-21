#include "Console.h"
void Console::SetLogFile(char * file)
{
	freopen_s(&inputStream, file, "r", stdin);
	freopen_s(&outputStream, file, "w", stdout);
	freopen_s(&errorStream, file, "w", stderr);
}

void Console::OpenConsole()
{
	AllocConsole();
	freopen_s(&inputStream, "conin$", "r", stdin);
	freopen_s(&outputStream, "conout$", "w", stdout);
	freopen_s(&errorStream, "conout$", "w", stderr);
}

void Console::CloseConsole()
{
	fclose(inputStream);
	fclose(outputStream);
	fclose(errorStream);
	FreeConsole();
}