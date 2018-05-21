#pragma once
#include "GuiElement.h"
#include "Renderer.h"
#include "Font.h"
#include <string.h>

class GuiText : public GuiElement
{
protected:
	std::string m_Text;
	Font* m_Font;
public:
	std::string GetText();
	Font* GetFont();
	void SetText(std::string data);
	void SetFont(Font* font);
	glm::vec2 GetSize() override;
	void OnClicked() {};
};