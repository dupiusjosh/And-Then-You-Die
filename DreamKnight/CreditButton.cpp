#include "CreditButton.h"
#include "DKEngine.h"
#include "CreditState.h"



CreditButton::CreditButton(size_t imageID) : GuiImage(imageID)
{
}


void CreditButton::OnClicked()
{
	DKEngine::GetInstance()->ChangeState(new CreditState(ObjectManager::GetInstance()));
}
