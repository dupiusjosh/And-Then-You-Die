#include "CreditState.h"
#include "GuiText.h"
#include "MainMenuButton.h"
#include "FontManager.h"

CreditState::CreditState(ObjectManager * objMan) //: m_ObjManager(objMan)
{
	m_ObjManager = objMan;
	Lines.push_back("Cody Ngo - Network Programmer");
	Lines.push_back("Tyler Peterson - Graphics Programmer");
	Lines.push_back("Josh Dupuis - Phyisics, and Gameplay");
	Lines.push_back("Holden Olivier - Data, and Gameplay");
	Lines.push_back("Cody Stanko - AI, and Levelgen");
	Lines.push_back("Troy Koppenhaver - Environment/Prop artist");
	Lines.push_back("Aurellio Ross Price - Character Artist");
	Lines.push_back("Glen Smith - Weapon Artist");
	Lines.push_back("Carol Quadrelli - Environment Textures");
	Lines.push_back("Jalissa Seals - Sound Effects");
	Lines.push_back("Kris Ducote - Mentor");
	Lines.push_back("Jason Hinders - Mentor");
	//Lines.push_back("SGX Staff");
	//Lines.push_back("NAME HERE");
}

void CreditState::Load()
{
	Canvas* m_Canvas = m_ObjManager->GetCanvas();

	GuiText* text = new GuiText();
	text->SetOffset(0.5f, 0.1f);
	text->SetAnchor(GuiElement::CENTER);
	text->SetFont(FontManager::GetInstance()->GetFont("fonts/times_new_roman_regular_48.xml"));
	text->SetText("And Then You Died");
	m_Canvas->AddElement(text);

	text = new GuiText();
	text->SetOffset(0.5f, 0.25f);
	text->SetAnchor(GuiElement::CENTER);
	text->SetFont(FontManager::GetInstance()->GetFont("fonts/times_new_roman_regular_20.xml"));
	text->SetText(Lines[0]);
	m_Canvas->AddElement(text);
	textboxes.push_back(text);

	text = new GuiText();
	text->SetOffset(0.5f, 0.40f);
	text->SetAnchor(GuiElement::CENTER);
	text->SetFont(FontManager::GetInstance()->GetFont("fonts/times_new_roman_regular_20.xml"));
	text->SetText(Lines[1]);
	m_Canvas->AddElement(text);
	textboxes.push_back(text);

	text = new GuiText();
	text->SetOffset(0.5f, 0.55f);
	text->SetAnchor(GuiElement::CENTER);
	text->SetFont(FontManager::GetInstance()->GetFont("fonts/times_new_roman_regular_20.xml"));
	text->SetText(Lines[2]);

	m_Canvas->AddElement(text);
	textboxes.push_back(text);

	text = new GuiText();
	text->SetOffset(0.5f, 0.7f);
	text->SetAnchor(GuiElement::CENTER);
	text->SetFont(FontManager::GetInstance()->GetFont("fonts/times_new_roman_regular_20.xml"));
	text->SetText(Lines[3]);

	m_Canvas->AddElement(text);
	textboxes.push_back(text);

	text = new GuiText();
	text->SetOffset(0.5f, 0.85f);
	text->SetAnchor(GuiElement::CENTER);
	text->SetFont(FontManager::GetInstance()->GetFont("fonts/times_new_roman_regular_20.xml"));
	text->SetText(Lines[4]);

	m_Canvas->AddElement(text);
	textboxes.push_back(text);

	size_t test_image = DataManager::GetInstance()->LoadTexture("Exit.png");

	MainMenuButton* button = new MainMenuButton(test_image);
	button->SetOffset(0.45f, 0.95f);
	button->SetAnchor(GuiElement::TOP |GuiElement::LEFT);
	m_Canvas->AddElement(button);

	printf("<<LOADING MENU FINISHED>>\n");
	FINISH();
}

void CreditState::StartLoad()
{
	loadThread = new std::thread(&CreditState::Load, this);

}

void CreditState::Update(ObjectManager * om, Renderer * rn, float deltaTime)
{
	timeSinceSwitch += deltaTime;
	if (timeSinceSwitch > timeTilSwitch)
	{
		//for the num of text boxes, change text to the next message.
		for (size_t i = 0; i < textboxes.size(); ++i)
		{
			if ((curIndx*textboxes.size()) + i < Lines.size())
				textboxes[i]->SetText(Lines[(curIndx*textboxes.size()) + i]);
			else
				textboxes[i]->SetText("");
		}
		curIndx++;
		timeSinceSwitch = 0;
	}
}

