#include "PickupControlComponent.h"
#include "NetworkDefines.h"
#include "NetServer.h"

PickupNetworkComponent::PickupNetworkComponent() : 
	m_MessageAvailable(false), m_ControlledPickup(nullptr)
{
}

void PickupNetworkComponent::OnUpdate(float deltaTime)
{
	PacketUpdate(deltaTime);
}

void PickupNetworkComponent::HandleMessage(RakNet::BitStream * bs)
{
	printf("PICKUP GOT MESSAGE\n");
	PickupMessage message;
	bs->Read<uint16_t>(message.MessageType);
	switch (message.MessageType)
	{
	case (MessageTypes::PickupSpawned):
	{
		glm::mat4 transform(1);
		glm::vec4 pos;
		uint16_t val;
		bs->Read<uint16_t>(val);
		bs->Read<glm::vec4>(transform[3]);
		//transform[3] = pos;
		m_ControlledPickup->Activate(transform, val);
		printf("Spawn pickup: %i\n", val);
		break;
	}
	case (MessageTypes::PickupDespawned):
	{
		m_ControlledPickup->Deactivate();
		break;
	}/*
	case (BaseNet_Comp::MessageTypes::Transform):
	{
		bs->Read<glm::mat4>(m_ControlledPickup->m_Transform);
	}*/
	default:
	{
		break;
	}
	}
}

void PickupNetworkComponent::PacketUpdate(const float & deltaTime)
{
	if (m_isServer)
	{
		elpsTime += deltaTime;
		if (elpsTime >= m_SyncRate)
		{
			/*RakNet::BitStream bs;
			bs.Write(GetNetworkID());
			bs.Write(BaseNet_Comp::MessageTypes::Transform);
			bs.Write<glm::mat4x4>(m_GameObject->m_Transform);
			m_Server->SendObjPacket(&bs);*/

			if (m_MessageAvailable)
			{
				RakNet::BitStream bs;
				bs.Write(GetNetworkID());
				bs.Write<uint16_t>(m_NextMessage.MessageType);

				if (m_NextMessage.MessageType == MessageTypes::PickupSpawned)
				{
					bs.Write<uint16_t>(m_NextMessage.PickupValue);
					bs.Write<glm::vec4>(m_NextMessage.Transform[3]);
					printf("PICKUP SEND MESSAGE\n");

				}
				m_Server->SendObjPacket(&bs);
				m_MessageAvailable = false;
			}

			elpsTime -= m_SyncRate;
		}
	}
}
