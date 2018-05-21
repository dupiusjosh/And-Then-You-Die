#include "GuiHealthbar.h"
#include "StatScript.h"

GuiHealthbar::GuiHealthbar(size_t imageID, StatScript* stats) : GuiImage(imageID)
{
	m_Stats = stats;
}

glm::vec2 GuiHealthbar::GetSize()
{
	return GuiImage::GetSize() * glm::vec2(m_Stats->GetHealthRatio(), 1.0f);
}

glm::vec2 GuiHealthbar::GetScale()
{
	return glm::vec2(m_Stats->GetHealthRatio(), 1.0f);
}

void GuiHealthbar::OnClicked()
{
	printf("Why the fuck did you click the health bar!?");
}

size_t GuiHealthbar::GetImage()
{
	return m_ImageID;
}
