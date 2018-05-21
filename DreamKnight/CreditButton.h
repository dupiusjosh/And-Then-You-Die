#pragma once
#include "GuiImage.h"
class CreditButton :
	public GuiImage
{
public:
	CreditButton(size_t imageID);
	void OnClicked();

};

