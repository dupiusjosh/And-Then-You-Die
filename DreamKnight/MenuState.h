#pragma once
#include "EngineState.h"
#include "GuiImage.h"
#include "GuiText.h"
#include "GuiTextbox.h"
#include "HostButton.h"

class MenuState :
	public EngineState
{
	GuiTextbox* guiText;
	GuiText* title;
	HostButton* button;

	void Load() override;
public:

	MenuState(ObjectManager * objMan);
	void StartLoad() override;
	virtual ~MenuState();
};

