#include "ProjectileComp.h"
#include "NetClient.h"
#include "NetServer.h"
#include "HitManagerComponent.h"


ProjectileComp::ProjectileComp(bool _IsPlayer)
{
	m_IsPlayers = _IsPlayer;
}

ProjectileComp::~ProjectileComp()
{

}

void ProjectileComp::OnUpdate(float dTime)
{
	m_forward = glm::vec4(normalize(my_bod->GetVelocity()),1.0f);
	curLife += dTime;
	if (curLife > maxLife)
	{
		m_GameObject->GetComponent<HitManager>()->ResetHits();
		//m_GameObject->GetComponent<SphereCollider>()->SetEnabled(false);
		m_GameObject->SetEnabled(false);
		curLife = 0.0f;
	}
	else
	{
		m_GameObject->GetComponent<RigidBodyComponent>()->SetVelocity(m_forward*10.0f);
		//m_GameObject->GetComponent<RigidBodyComponent>()->SetVelocity(m_forward*500.0f*dTime);
	}
}

void ProjectileComp::OnStart()
{
	curLife = 0.0f;
	maxLife = 0.5f;
	my_bod = m_GameObject->GetComponent<RigidBodyComponent>();
}

void ProjectileComp::ServerUpdate(const float& dTime)
{

}

void ProjectileComp::ClientUpdate(const float& dTime)
{

}

void ProjectileComp::HandleMessage(RakNet::BitStream *bs)
{

}