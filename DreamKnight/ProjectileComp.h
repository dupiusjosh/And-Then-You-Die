#pragma once
#include "BaseNet_Comp.h"
#include "glm.h"
#include "RigidBodyComponent.h"

class ProjectileComp : public BaseNet_Comp
{
private:
	bool m_IsPlayers;
	float m_Speed = 3.0f;
	int m_Damage = 0;
	float maxLife;// = .50f;
	float curLife;// = 0.0f;

	RigidBodyComponent * my_bod;
	glm::vec4 m_forward;
public:
	ProjectileComp(bool _IsPlayers);
	~ProjectileComp();

	void SetSpeed(float f) { m_Speed = f; }
	void OnUpdate(float deltaTime);
	void OnStart();
	void ServerUpdate(const float& deltaTime);
	void ClientUpdate(const float& deltaTime);
	void HandleMessage(RakNet::BitStream *bs);
};