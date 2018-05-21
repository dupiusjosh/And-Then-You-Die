#pragma once
#include "GuiImage.h"
class ExitButton :
	public GuiImage
{
public:
	ExitButton(size_t imageID);
	//~ExitButton();
	void OnClicked();

};

