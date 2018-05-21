#include "NetChestComponent.h"
#include "PickupManager.h"
#include "DataManager.h"
#include "NetworkDefines.h"
#include "NetServer.h"
#include "NetClient.h"
#include "ItemRef.h"
#include "AnimationComponent.h"

#define OpenState 0ui8
#define ClosedState 1ui8

NetChestComp::NetChestComp() : BaseNet_Comp(), m_MessageAvailable(true)//, m_NextMessage(MessageTypes::RequestTransform)
{
	m_State = ClosedState;
	m_ChestDropValue = rand() % (uint16_t)ItemIDs::numIds;
}

void NetChestComp::DropItem(glm::vec4 pos)
{
	if (m_State == ClosedState)
	{
		if (m_isServer)
		{
			//glm::vec4 result = pos - m_GameObject->m_Transform[3];
			//result = glm::normalize(result);
			//result *= 3;

			glm::vec4 result = m_GameObject->m_Transform * glm::vec4(0, 0, 1, 1);

			//m_ChestDropValue = (uint16_t)ItemIDs::ChickenDinner;
			PickupManager* pm = PickupManager::GetInstance();
			pm->SpawnItemPickup(/*m_GameObject->m_Transform[3] +*/ result, m_ChestDropValue);
			//pm->SpawnHealthPickup(m_GameObject->m_Transform[3]);// +result);
			SetState(OpenState);
			Utils::PrintWarning("I'm a Server Chest, Trying to Drop Items!\n");
		}
		else
		{
			Utils::PrintWarning("I'm a Client Chest, Trying to Drop Items!\n");
		}
	}
}

void NetChestComp::SetState(uint16_t state)
{
	if (state != m_State)
	{
		m_State = state;


		
		AnimationComponent* ac = m_GameObject->GetComponent<AnimationComponent>();
		//SphereCollider* sc = m_GameObject->GetComponent<SphereCollider>();
		switch (m_State)
		{
			case OpenState:
			{
				if (ac) ac->SetAnimationState(SingleAnimating);

				if (IsServer())
				{
					RakNet::BitStream bs;
					bs.Write(GetNetworkID());
					bs.Write<uint16_t>(ChestOpened);
					m_Server->SendObjPacket(&bs, UNRELIABLE);
				}

				break;
			}
			case ClosedState:
			{
				if (ac) ac->SetAnimationState(OneFrameAnimating);

				if (IsServer())
				{
					RakNet::BitStream bs;
					bs.Write(GetNetworkID());
					bs.Write<uint16_t>(ChestOpened);
					m_Server->SendObjPacket(&bs, UNRELIABLE);
				}

				break;
			}
			default:
				break;
		}

	}
}

void NetChestComp::HandleMessage(RakNet::BitStream * bs)
{
	MessageTypes type;
	bs->Read<MessageTypes>(type);

	switch (type)
	{
		case (MessageTypes::Transform):
		{

			glm::mat4x4 mat;
			bs->Read<glm::mat4x4>(mat);
			m_GameObject->m_Transform = mat;
			break;
		}
		case (MessageTypes::ChestClosed):
		{
			SetState(ClosedState);
			break;
		}
		case (MessageTypes::ChestOpened):
		{
			SetState(OpenState);
			break;
		}
		//case (MessageTypes::RequestTransform):
		//{
		//	m_NextMessage = BaseNet_Comp::MessageTypes::Transform;
		//}
	}
}


void NetChestComp::PacketUpdate(const float & deltaTime)
{

	if (m_isServer)
	{
		elpsTime += deltaTime;
		if (elpsTime >= m_SyncRate)
		{
			elpsTime -= m_SyncRate;

			RakNet::BitStream bs;
			bs.Write(GetNetworkID());
			bs.Write(MessageTypes::Transform);
			bs.Write<glm::mat4>(m_GameObject->m_Transform);
			m_Server->SendObjPacket(&bs);
			
			//{
			//	RakNet::BitStream bs;
			//	bs.Write(GetNetworkID());
			//	bs.Write<uint16_t>(StateChanged);
			//	bs.Write<uint16_t>(m_State);
			//	//bs.Write<PlayerStates>(m_CurrState);
			//	m_Server->SendObjPacket(&bs, UNRELIABLE);
			//}
		}
	}/*
	else if (m_MessageAvailable)
	{
		if (m_NextMessage == MessageTypes::RequestTransform)
		{
			RakNet::BitStream bs;
			bs.Write(GetNetworkID());
			bs.Write<uint16_t>(m_NextMessage);
			m_Client->SendObjPacket(&bs);
			m_MessageAvailable = false;
		}
	}*/
}

void NetChestComp::OnUpdate(float deltaTime)
{
	PacketUpdate(deltaTime);
}
