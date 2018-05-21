#include "GuiElement.h"
#include "Renderer.h"

GuiElement::~GuiElement()
{
}

glm::vec2 GuiElement::GetOffset()
{
	glm::vec2 local_offset = GetSize();
	if (m_Anchor & LEFT)
		local_offset.x = 0;
	else if (!(m_Anchor & RIGHT))
		local_offset.x *= 0.5f;
	if (m_Anchor & TOP)
		local_offset.y = 0;
	else if (!(m_Anchor & BOTTOM))
		local_offset.y *= 0.5f;
	Renderer* renderer = Renderer::GetInstance();
	local_offset /= glm::vec2(renderer->GetWidth(), renderer->GetHeight());
	return m_Offset - local_offset;
}
