#pragma once
#include "GuiImage.h"

class HostButton : public GuiImage
{
protected:
	size_t m_ImageID;
public:
	GuiTextbox* textbox;
	HostButton(size_t imageID);
	void OnClicked();
};