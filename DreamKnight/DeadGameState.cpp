#include "DeadGameState.h"


#include "Object.h"
#include "TempSceneSwitch.h"

#include "FontManager.h"
#include "HostButton.h"
#include "ExitButton.h"

#include "GuiImage.h"
#include "GuiText.h"
#include "GuiTextbox.h"
#include "MainMenuButton.h"
void DeadGameState::Load()
{

	Canvas* m_Canvas = m_ObjManager->GetCanvas();
	//m_ObjManager->AddObject(m_Canvas);

	GuiText* title = new GuiText();
	title->SetOffset(0.5f, 0.1f);
	title->SetAnchor(GuiElement::CENTER);
	title->SetFont(FontManager::GetInstance()->GetFont("fonts/times_new_roman_regular_48.xml"));
	title->SetText("And Then You Died");
	m_Canvas->AddElement(title);

	//"Play" Button
	size_t test_image = DataManager::GetInstance()->LoadTexture("Exit.png");
	MainMenuButton* button = new MainMenuButton(test_image);
	button->SetOffset(0.5f, 0.5f);
	button->SetAnchor(GuiElement::CENTER);
	m_Canvas->AddElement(button);


	printf("<<LOADING Dead FINISHED>>\n");
	FINISH();
}

DeadGameState::DeadGameState(ObjectManager * objMan)// : EngineState::m_ObjManager(objMan)
{
	//EngineState::EngineState(objMan);
	m_ObjManager = objMan;
}


DeadGameState::~DeadGameState()
{
}
