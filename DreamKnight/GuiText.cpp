#include "GuiText.h"

std::string GuiText::GetText()
{
	return m_Text;
}

Font* GuiText::GetFont()
{
	return m_Font;
}

void GuiText::SetText(std::string data)
{
	m_Text = data;
}

void GuiText::SetFont(Font * font)
{
	m_Font = font;
}

glm::vec2 GuiText::GetSize()
{
	return glm::vec2(m_Font->getWidthOfString(m_Text), m_Font->getHeightOfString(m_Text));
}
