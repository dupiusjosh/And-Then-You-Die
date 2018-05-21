#pragma once

#define _USE_MATH_DEFINES
#include "glm.h"
#include <cmath>
#include <mutex>
#include "Object.h"
#include "Component.h"
#include "Mesh.h"

//using namespace glm;

class RigidBodyComponent : public Component
{
private:
	float m_Mass;
	float m_MassInverse;
	glm::vec3 m_Gravity;
	glm::vec3 m_Weight;
	glm::vec3 m_Velocity;
	glm::vec3 m_Acceleration;
	glm::vec3 m_PrevAcceleration;
	glm::vec3 m_ConstantForce;
	glm::vec3 m_ImpulseForce;
	bool m_IsGravityApplied;
	bool m_IsKinematic;
	bool m_IsFrozen;
	std::mutex m_Mutex;
	glm::vec3 m_TotalForces;



public:
	RigidBodyComponent();
	RigidBodyComponent(float mass, glm::vec3 gravity, glm::vec3 velocity, glm::vec3 acceleration, bool isGravityApplied, bool isKinematic);

	void OnUpdate(float deltaTime);
	void AddConstantForce(glm::vec3 force);
	void AddImpulseForce(glm::vec3 force);
	void CancelAcceleration(glm::vec3 direction);
	void CancelVelocity(glm::vec3 direction);

	void AddVelocity(glm::vec3 velocity);

	// Getters
	const float GetMass();
	const glm::vec3 GetGravity();
	const glm::vec3 GetVelocity();
	const glm::vec3 GetAcceleration();
	const glm::vec3 GetConstantForce();
	const glm::vec3 GetTotalForce();
	const bool GetIsGravityApplied();
	const bool GetIsKinematic();
	const bool GetIsFrozen();

	// Setters
	void SetMass(float mass);
	void SetGravity(glm::vec3 gravity);
	void SetVelocity(glm::vec3 velocity);
	void SetConstantForce(glm::vec3 constantForce);
	void SetIsGravityApplied(bool isGravityApplied);
	void SetIsKinematic(bool isKinematic);
	void SetIsFrozen(bool stopMovement);

	~RigidBodyComponent();
};