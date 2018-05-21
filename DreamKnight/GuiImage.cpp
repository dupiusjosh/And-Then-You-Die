#include "GuiImage.h"
#include "DKEngine.h"
#include "GameState.h"

GuiImage::GuiImage(size_t imageID)
{
	m_ImageID = imageID;
}

glm::vec2 GuiImage::GetSize()
{
	Texture* tex = Renderer::GetInstance()->GetTexture(m_ImageID);
	return glm::vec2(tex->width, tex->height);
}

glm::vec2 GuiImage::GetScale()
{
	return glm::vec2(1);
}

void GuiImage::OnClicked()
{}

size_t GuiImage::GetImage()
{
	return m_ImageID;
}
