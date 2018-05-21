#pragma once
#include "GuiImage.h"
class MainMenuButton :
	public GuiImage
{
public:
	MainMenuButton(size_t imageID);
	void OnClicked() override;
};

