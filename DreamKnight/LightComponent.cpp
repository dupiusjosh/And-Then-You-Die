#include "LightComponent.h"
#include "Renderer.h"

void LightComponent::SetLightType(LightType lt)
{
	m_Offset.w = (float)lt;
}

void LightComponent::SetDirection(glm::vec3 dir)
{
	m_Dir.x = dir.x;
	m_Dir.y = dir.y;
	m_Dir.z = dir.z;
}

void LightComponent::SetDistance(float distance)
{
	m_Dir.w = distance;
}

void LightComponent::OnUpdate(float deltaTime)
{
	if (m_GameObject->m_Creator != 0)
	{
		Light light;
		light.m_Direction = m_Dir;

		glm::vec4 temp = m_Offset;
		temp.w = 1.0f;
		temp = m_GameObject->GetWorldMatrix() * temp;
		temp.w = m_Offset.w;
		
		light.m_Position = temp;
		light.m_Color = m_Color;
		Renderer::GetInstance()->AddLight(light);
	}
}

void LightComponent::SetOffset(glm::vec3 pos)
{
	m_Offset.x = pos.x;
	m_Offset.y = pos.y;
	m_Offset.z = pos.z;
}

void LightComponent::SetColor(glm::vec4 color)
{
	m_Color = color;
}