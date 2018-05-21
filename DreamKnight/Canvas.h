#pragma once
#include <vector>
#include "Object.h"

class GuiElement;

class Canvas : public Object
{
	std::vector<GuiElement*> m_Elements;
	GuiElement* m_SelectedElement;
	GuiElement* m_FocusedElement;

public:
	~Canvas();
	void AddElement(GuiElement* element);
	inline std::vector<GuiElement*> GetElements() { return m_Elements; };
	void Update(float deltaTime);
	GuiElement* CheckTarget(float x, float y);
};