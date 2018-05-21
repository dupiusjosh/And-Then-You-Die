#pragma once
#include "BaseNet_Comp.h"
#include <glm/mat4x4.hpp>

class Pickup;

struct PickupNetworkComponent : BaseNet_Comp
{
	enum MessageTypes : uint16_t
	{
		PickupSpawned = BaseNet_Comp::MessageTypes::CustomMessageStart,
		PickupDespawned,
	};

	struct PickupMessage
	{
		uint16_t MessageType;
		uint16_t PickupValue;
		glm::mat4 Transform;
	};
	
	PickupMessage m_NextMessage;
	bool m_MessageAvailable;

	Pickup* m_ControlledPickup;

	PickupNetworkComponent();

	void OnUpdate(float deltaTime);
	void HandleMessage(RakNet::BitStream *bs);
	void PacketUpdate(const float& deltaTime);
};