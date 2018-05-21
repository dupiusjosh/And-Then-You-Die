#pragma once
#include "GuiImage.h"
#include "Renderer.h"
#include "StatScript.h"
class GuiHealthbar : public GuiImage
{
protected:
	size_t m_ImageID;
	StatScript* m_Stats;
public:
	GuiHealthbar::GuiHealthbar(size_t imageID, StatScript* stats);
	glm::vec2 GetSize() override;
	glm::vec2 GetScale() override;
	void OnClicked();
	size_t GetImage();

};