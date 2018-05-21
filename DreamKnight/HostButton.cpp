#include "HostButton.h"
#include "DKEngine.h"
#include "GameState.h"
#include "GuiTextbox.h"

HostButton::HostButton(size_t imageID) : GuiImage(imageID)
{}

void HostButton::OnClicked()
{
	DKEngine* engine = DKEngine::GetInstance();
	GameState::PassedData* data = new GameState::PassedData();
	if (textbox)
	{
		char* c = textbox->GetText();
		strcpy_s(data->ip, c);
	}
	engine->ChangeState(new GameState(engine->m_ObjManager, data));
}
