#pragma once
#include "Component.h"
#include "glm.h"

enum class LightType
{
	None = 0,
	Ambient,
	Directional,
	Point,
};

class LightComponent : public Component
{
	glm::vec4 m_Offset, m_Dir, m_Color;
public:
	void SetLightType(LightType lt);
	void SetDirection(glm::vec3 dir);
	void SetDistance(float distance);
	void OnUpdate(float deltaTime);
	void SetOffset(glm::vec3 pos);
	void SetColor(glm::vec4 color);
};