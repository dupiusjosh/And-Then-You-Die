#include "RigidBodyComponent.h"

// Constructors
RigidBodyComponent::RigidBodyComponent()
{
	m_Mass = 0.0f;
	m_MassInverse = 0.0f;
	m_Gravity = glm::vec3(0.0f, -9.81f, 0.0f);
	m_Weight = m_Mass * m_Gravity;
	m_Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	m_Acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	m_PrevAcceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	m_ConstantForce = glm::vec3(0.0f, 0.0f, 0.0f);
	m_ImpulseForce = glm::vec3(0.0f, 0.0f, 0.0f);
	m_IsGravityApplied = true;
	m_IsKinematic = true;
	m_IsFrozen = false;
}

RigidBodyComponent::RigidBodyComponent(float mass, glm::vec3 gravity, glm::vec3 velocity, glm::vec3 constantForce, bool isGravityApplied, bool isKinematic)
{
	m_Mass = mass;
	m_Gravity = gravity;
	m_Weight = m_Mass * m_Gravity;
	m_Velocity = velocity;
	m_ConstantForce = constantForce;
	m_IsGravityApplied = isGravityApplied;
	m_IsKinematic = isKinematic;

	m_MassInverse = 1.0f / mass;
	m_Acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	m_PrevAcceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	m_ImpulseForce = glm::vec3(0.0f, 0.0f, 0.0f);
	m_IsFrozen = false;
}

void RigidBodyComponent::OnUpdate(float deltaTime)
{
	if (!m_IsFrozen)
	{
		if (deltaTime <= FLT_EPSILON)
			return;
		m_TotalForces = m_ConstantForce + m_Weight + (m_ImpulseForce * (1 / deltaTime));
		m_ImpulseForce = glm::vec3(0, 0, 0);
		m_Acceleration = m_TotalForces * m_MassInverse;
		m_Velocity += (m_Acceleration + m_PrevAcceleration) * 0.5f * deltaTime;
		m_PrevAcceleration = m_Acceleration;

		//m_TotalForces = m_ConstantForce + m_Weight;// +(m_ImpulseForce / deltaTime);
		//m_Acceleration = m_TotalForces * m_MassInverse;
		//m_Velocity += (m_Acceleration + m_PrevAcceleration) * 0.5f * deltaTime;
		//m_ImpulseForce = glm::vec3(0, 0, 0);

		//m_PrevAcceleration = m_Acceleration;

		//m_PrevAcceleration = m_Acceleration + (m_PrevAcceleration - m_Acceleration) * (1.0f - deltaTime);
		//m_GameObject->m_Transform = translate(m_GameObject->m_Transform, m_Velocity * deltaTime);

		if (isnan(m_Velocity.y))
			printf("STOP\n");
		m_GameObject->m_Transform[3].x += m_Velocity.x * deltaTime;
		m_GameObject->m_Transform[3].y += m_Velocity.y * deltaTime;
		m_GameObject->m_Transform[3].z += m_Velocity.z * deltaTime;

	}
}

void RigidBodyComponent::AddConstantForce(glm::vec3 force)
{
	m_Mutex.lock();
	m_ConstantForce += force;
	m_Mutex.unlock();
}

void RigidBodyComponent::AddImpulseForce(glm::vec3 force)
{
	m_Mutex.lock();
	m_ImpulseForce = force;
	m_Mutex.unlock();
}

void RigidBodyComponent::CancelAcceleration(glm::vec3 direction)
{
	for (glm::length_t i = 0; i < 3; i++)
	{
		if (direction[i] != 0)
		{
			m_Acceleration[i] = 0;
			m_PrevAcceleration[i] = 0;
		}
	}
}

void RigidBodyComponent::CancelVelocity(glm::vec3 direction)
{
	for (glm::length_t i = 0; i < 3; i++)
	{
		if (direction[i] != 0)
		{
			m_Velocity[i] = 0;
		}
	}
}

void RigidBodyComponent::AddVelocity(glm::vec3 velocity)
{
	m_Velocity += velocity;
}


// Getters
const float RigidBodyComponent::GetMass()
{
	float mass;

	m_Mutex.lock();
	mass = m_Mass;
	m_Mutex.unlock();

	return mass;
}

const glm::vec3 RigidBodyComponent::GetGravity()
{
	glm::vec3 gravity;

	m_Mutex.lock();
	gravity = m_Gravity;
	m_Mutex.unlock();

	return gravity;
}

const glm::vec3 RigidBodyComponent::GetVelocity()
{
	glm::vec3 velocity;

	m_Mutex.lock();
	velocity = m_Velocity;
	m_Mutex.unlock();

	return velocity;
}

const glm::vec3 RigidBodyComponent::GetAcceleration()
{
	glm::vec3 acceleration;

	m_Mutex.lock();
	acceleration = m_Acceleration;
	m_Mutex.unlock();

	return acceleration;
}

const glm::vec3 RigidBodyComponent::GetConstantForce()
{
	glm::vec3 constantForce;

	m_Mutex.lock();
	constantForce = m_ConstantForce;
	m_Mutex.unlock();

	return constantForce;
}

const glm::vec3 RigidBodyComponent::GetTotalForce()
{
	glm::vec3 totalForce;

	m_Mutex.lock();
	totalForce = m_TotalForces;
	m_Mutex.unlock();

	return totalForce;
}

const bool RigidBodyComponent::GetIsGravityApplied()
{
	bool isGravityApplied;

	m_Mutex.lock();
	isGravityApplied = m_IsGravityApplied;
	m_Mutex.unlock();

	return isGravityApplied;
}

const bool RigidBodyComponent::GetIsKinematic()
{
	bool isKinematic;

	m_Mutex.lock();
	isKinematic = m_IsKinematic;
	m_Mutex.unlock();

	return isKinematic;
}

const bool RigidBodyComponent::GetIsFrozen()
{
	bool isFrozen;

	m_Mutex.lock();
	isFrozen = m_IsFrozen;
	m_Mutex.unlock();

	return isFrozen;
}


// Setters
void RigidBodyComponent::SetMass(float mass)
{
	m_Mutex.lock();
	m_Mass = mass;
	m_MassInverse = 1 / mass;
	m_Weight = m_Mass * m_Gravity;
	m_Mutex.unlock();
}

void RigidBodyComponent::SetGravity(glm::vec3 gravity)
{
	m_Mutex.lock();
	m_Gravity = gravity;
	m_Weight = m_Mass * m_Gravity;
	m_Mutex.unlock();
}

void RigidBodyComponent::SetVelocity(glm::vec3 velocity)
{
	if (isnan(velocity.y))
		printf("STOP\n");
	m_Mutex.lock();
	m_Velocity = velocity;
	m_Mutex.unlock();
}

void RigidBodyComponent::SetConstantForce(glm::vec3 constantForce)
{
	m_Mutex.lock();
	m_ConstantForce = constantForce;
	m_Mutex.unlock();
}

void RigidBodyComponent::SetIsGravityApplied(bool isGravityApplied)
{
	m_Mutex.lock();
	m_IsGravityApplied = isGravityApplied;
	m_Mutex.unlock();
}

void RigidBodyComponent::SetIsKinematic(bool isKinematic)
{
	m_Mutex.lock();
	m_IsKinematic = isKinematic;
	m_Mutex.unlock();
}

void RigidBodyComponent::SetIsFrozen(bool stopMovement)
{
	m_Mutex.lock();
	m_IsFrozen = stopMovement;
	m_Mutex.unlock();
}

// Destructor
RigidBodyComponent::~RigidBodyComponent()
{
}