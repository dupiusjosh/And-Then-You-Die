#pragma once
#include "GuiElement.h"
#include "Renderer.h"
class GuiImage : public GuiElement
{
protected:
	size_t m_ImageID;
public:
	GuiImage(size_t imageID);
	glm::vec2 GetSize() override;
	virtual glm::vec2 GetScale();
	void OnClicked();
	size_t GetImage();

};