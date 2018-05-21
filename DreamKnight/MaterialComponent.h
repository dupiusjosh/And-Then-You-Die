#pragma once
#include "Component.h"
#include "glm.h"

struct MaterialComponent : public Component
{
	glm::vec4 m_Color = glm::vec4(1,1,1,1);

public:
	MaterialComponent() {}
	void OnUpdate(float deltaTime){}
	void SetColor(glm::vec4 color) { m_Color = color; };
};