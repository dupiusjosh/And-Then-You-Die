#include "MenuState.h"

#include "Object.h"
#include "TempSceneSwitch.h"

#include "FontManager.h"
#include "HostButton.h"
#include "ExitButton.h"
#include "CreditButton.h"
void MenuState::Load()
{
	printf("<<LOADING MENU >>\n");
	
	Canvas* m_Canvas = m_ObjManager->GetCanvas();
	//m_ObjManager->AddObject(m_Canvas);

	size_t bg = DataManager::GetInstance()->LoadTexture("MainMenu/BG_solo.jpg");
	GuiImage* bgImage = new GuiImage(bg);
	bgImage->SetOffset(0.0f, 0.0f);
	bgImage->SetAnchor(GuiElement::AnchorPoint::LEFT | GuiElement::AnchorPoint::TOP);
	m_Canvas->AddElement(bgImage);


	GuiTextbox* guiText = new GuiTextbox();
	guiText->SetOffset(0.5f, 0.4f);
	guiText->SetAnchor(GuiElement::CENTER);
	guiText->SetFont(FontManager::GetInstance()->GetFont("fonts/times_new_roman_regular_20.xml"));
	guiText->SetImage(DataManager::GetInstance()->LoadTexture("textbox.png"));
	m_Canvas->AddElement(guiText);

	//GuiText* title = new GuiText();
	//title->SetOffset(0.5f, 0.1f);
	//title->SetAnchor(GuiElement::CENTER);
	//title->SetFont(FontManager::GetInstance()->GetFont("fonts/times_new_roman_regular_48.xml"));
	//title->SetText("And Then You Die");
	//m_Canvas->AddElement(title);

	//"Play" Button
	size_t test_image = DataManager::GetInstance()->LoadTexture("MainMenu/BlueButton Play.png");
	HostButton* button = new HostButton(test_image);
	button->SetOffset(0.1f, 0.4f);
	button->SetAnchor(GuiElement::AnchorPoint::LEFT | GuiElement::AnchorPoint::TOP);
	button->textbox = guiText;
	m_Canvas->AddElement(button);

	//BlueButton Credits
	size_t cred = DataManager::GetInstance()->LoadTexture("MainMenu/BlueButton Credits.png");
	CreditButton * cbutton = new CreditButton(cred);
	cbutton->SetOffset(0.09f, 0.58f);
	cbutton->SetAnchor(GuiElement::AnchorPoint::LEFT | GuiElement::AnchorPoint::TOP);
	m_Canvas->AddElement(cbutton);

	size_t exit = DataManager::GetInstance()->LoadTexture("MainMenu/BlueButton Exit.png");
	ExitButton* ebutton = new ExitButton(exit);
	ebutton->SetOffset(0.1f, 0.75f);
	ebutton->SetAnchor(GuiElement::AnchorPoint::LEFT | GuiElement::AnchorPoint::TOP);
	m_Canvas->AddElement(ebutton);



	printf("<<LOADING MENU FINISHED>>\n");
	FINISH();
}


MenuState::MenuState(ObjectManager * objMan) //: m_ObjManager(objMan)
{
	m_ObjManager = objMan;
}

void MenuState::StartLoad()
{
	loadThread = new std::thread(&MenuState::Load, this);
}

MenuState::~MenuState()
{
}
