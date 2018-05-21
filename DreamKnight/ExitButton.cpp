#include "ExitButton.h"
#include "DKEngine.h"


ExitButton::ExitButton(size_t imageID) : GuiImage(imageID)
{
}


void ExitButton::OnClicked()
{
	PostQuitMessage(0);
	SendMessage(0, WM_QUIT, 0, 0);
}
