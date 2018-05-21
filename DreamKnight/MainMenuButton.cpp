#include "MainMenuButton.h"

#include "DKEngine.h"
#include "MenuState.h"
MainMenuButton::MainMenuButton(size_t imageID) : GuiImage(imageID)
{
}

void MainMenuButton::OnClicked()
{
	//Whatever. chn
	DKEngine::GetInstance()->ChangeState(new MenuState(ObjectManager::GetInstance()));
}


