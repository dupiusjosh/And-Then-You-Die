#include "PickupManager.h"
#include "MeshComponent.h"
#include "glm.h"
#include "Pickup.h"
#include "NetServer.h"
namespace
{
	PickupManager* ServerManager = nullptr;
}

PickupManager::PickupManager() : m_Server(nullptr), m_CurrentHealthPickup(0)
{
	for (auto& p : m_HealthPickups)
	{
		p = nullptr;
	}
}


PickupManager::~PickupManager()
{
	ServerManager = nullptr;

}

PickupManager * PickupManager::GetInstance()
{
		if (ServerManager == nullptr)
		{
			ServerManager = new PickupManager();
		}
		return ServerManager;
}

void PickupManager::SetServer(NetServer * serv)
{
	m_Server = serv;
}

void PickupManager::Init()
{
	m_HealthPickupValue = 20;
	m_CurrentHealthPickup = 0;

	m_ItemPickups.reserve(10);

	for (Pickup*& p : m_HealthPickups)
	{
		if (m_Server)
		{
			p = (Pickup*)m_Server->AddNetworkedObject(ObjectCreation::Types::PickupsHP, 0);
			p->m_Value.m_HealthValue = m_HealthPickupValue;
			p->m_Type = PickupType::HealthPickup;
			p->m_Collected = false;
		}
	}
}

void PickupManager::Reset()
{
	m_ItemPickups.clear();
	Init();
}

void PickupManager::SpawnHealthPickup(glm::vec3 pos)
{
	auto& ref = m_HealthPickups[m_CurrentHealthPickup];
	ref->Activate(glm::translate(glm::mat4(), pos), m_HealthPickupValue);

	if (++m_CurrentHealthPickup >= MaxHealthPickups)
	{
		m_CurrentHealthPickup = 0;
	}
}

#pragma optimize("", off)
void PickupManager::SpawnItemPickup(glm::vec3 pos, uint16_t itemId)
{
	//Get or make an unused item pickup
	Pickup* ref = nullptr;
	for (auto p : m_ItemPickups)
	{
		if (p->isEnabled() == false)
		{
			ref = p;
		}
	}
	if (ref == nullptr)
	{
		printf("MAKE NEW PICKUP\n");

		ref = (Pickup*)m_Server->AddNetworkedObject(ObjectCreation::Types::PickupsItem, 0);
		m_ItemPickups.push_back(ref);

		RakNet::BitStream bsOut;
		bsOut.Write((RakNet::MessageID)sRegisterObject);
		bsOut.Write<ObjectCreation::Types>(ObjectCreation::Types::PickupsItem);
		bsOut.Write<glm::mat4>(ref->m_Transform);
		bsOut.Write<NetworkID>(ref->GetComponent<BaseNet_Comp>()->GetNetworkID());
		bsOut.Write<uint32_t>(0);

		m_Server->m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}
	
	ref->Activate(glm::translate(glm::mat4(), pos), itemId);
}
#pragma optimize("", on)

void PickupManager::SpawnEffectPickup(glm::vec3 pos, uint16_t itemId)
{
	//Get or make an unused item pickup
	Pickup* ref = nullptr;
	for (auto p : m_EffectPickups)
	{
		if (p->isEnabled() == false)
		{
			ref = p;
		}
	}
	if (ref == nullptr)
	{
		ref = (Pickup*)m_Server->AddNetworkedObject(ObjectCreation::Types::PickupsEffect, 0);
		m_EffectPickups.push_back(ref);

		RakNet::BitStream bsOut;
		bsOut.Write((RakNet::MessageID)sRegisterObject);
		bsOut.Write(ObjectCreation::Types::PickupsEffect);
		bsOut.Write(ref->m_Transform);
		bsOut.Write(ref->GetComponent<BaseNet_Comp>()->GetNetworkID());
		bsOut.Write(0);
		m_Server->m_Connection.Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}

	ref->Activate(glm::translate(glm::mat4(), pos), itemId);
}

void PickupManager::GarbageCollectPickups()
{
	size_t LastGoodIndex = m_ItemPickups.max_size();
	size_t i = 0;
	for (i = m_ItemPickups.size(); i >= 0; --i)
	{
		if (m_ItemPickups[i]->m_Collected == false && LastGoodIndex == m_ItemPickups.max_size())
		{
			LastGoodIndex = i;
		}
		else if (m_ItemPickups[i]->m_Collected && LastGoodIndex != m_ItemPickups.max_size())
		{
			m_ItemPickups[i]->Swap(m_ItemPickups.back());
			if (LastGoodIndex != 0) { --LastGoodIndex; }
		}
		if (i == 0) { break; }
	}
	LastGoodIndex = m_EffectPickups.max_size();
	for (i = m_EffectPickups.size(); i >= 0; --i)
	{
		if (m_EffectPickups[i]->m_Collected == false && LastGoodIndex == m_EffectPickups.max_size())
		{
			LastGoodIndex = i;
		}
		else if (m_EffectPickups[i]->m_Collected && LastGoodIndex != m_EffectPickups.max_size())
		{
			m_EffectPickups[i]->Swap(m_EffectPickups.back());
			if (LastGoodIndex != 0) { --LastGoodIndex; }
		}
		if (i == 0) { break; }
	}
}
