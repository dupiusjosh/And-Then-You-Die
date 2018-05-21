#pragma once
#include "Component.h"
#include "glm.h"
#include "Renderer.h"
#include "InputManager.h"
class StatScript;
class NetPlayerComponent;
class ThirdPersonCameraComponent : public Component
{
	bool m_freeRoam = true;
	bool m_Spectate = false;
	glm::vec3 m_position;
	Object * m_target;
	Renderer * m_render;
	glm::mat4 m_Camera;
	InputManager * m_activeIM = nullptr;
	float m_MoveSpeed;
	float m_RotateSpeed;
	glm::vec3 CameraOffsetPos;
	glm::vec3 LookAtOffsetPos;
	float rotX, rotY;
	float m_RotYFloor, m_RotYCap;
	
	float m_timer = 1.0f;
	NetPlayerComponent * playerScript;
	std::vector<Object*> m_players;
	unsigned int m_SpecTargetIndx = 0;
	//unsigned int prevmouseX, prevmouseY;
	//glm::vec3 offset;
public:
	ThirdPersonCameraComponent();
	~ThirdPersonCameraComponent();

	void DebugUpdateMouse(float deltaTime);// , float x, float y);
	void UpdateMouse(float deltaTime);// , float x, float y);
	void ChangeTarget(float deltaTime);
	void OnUpdate(float deltaTime);
	void OnStart();
	glm::mat4& GetCamera() { return m_Camera; };

	//void CheckStaticObjects(vec4 point, vec4 direction, ObjectManager* manager);
};

