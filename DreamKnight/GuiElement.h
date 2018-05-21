#pragma once
#include "glm.h"

//using namespace glm;

class GuiElement
{
public:
	enum AnchorPoint
	{
		CENTER = 0,
		TOP = 1,
		BOTTOM = 2,
		RIGHT = 4,
		LEFT = 8,
	};
protected:
	glm::vec2 m_Offset;
	glm::vec2 m_Size;
	size_t m_Shader;

	uint8_t m_Anchor = TOP | LEFT;

public:
	virtual ~GuiElement();
	glm::vec2 GetOffset();
	inline void SetOffset(float x, float y) { m_Offset.x = x; m_Offset.y = y; };
	inline void SetAnchor(uint8_t anchor) { m_Anchor = anchor; };
	virtual glm::vec2 GetSize() { return glm::vec2(0, 0); };
	virtual void OnClicked() = 0;
	virtual bool Focusable() { return false; };
	virtual void UnFocus() {};
	virtual void Update() {};
};