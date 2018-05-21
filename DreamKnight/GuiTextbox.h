#pragma once
#include "GuiElement.h"
#include "Renderer.h"
#include "Font.h"
#include <string.h>

class GuiTextbox : public GuiElement
{
protected:
	char* m_Text = (char*)calloc(512, sizeof(char));
	int m_Pos = 0;
	Font* m_Font;
	size_t m_ImageID = 0;

	float rep = 0;

	bool focused = false;
	void AddChar(char c);

public:
	~GuiTextbox();

	char* GetText();
	Font* GetFont();
	void SetText(char* data);
	void SetFont(Font* font);
	void SetImage(size_t id);
	size_t GetImage();
	glm::vec2 GetSize() override;
	glm::vec2 GetTextOffset();
	void OnClicked();
	bool Focusable() { return true; }
	void UnFocus();
	void Backspace();
	void Update();
};