#pragma once
#include "BaseNet_Comp.h"
#include <glm/vec4.hpp>

struct Mesh;

struct NetChestComp : public BaseNet_Comp
{
	//enum MessageTypes : uint16_t
	enum MessageTypes : uint16_t
	{
		ChestOpened = BaseNet_Comp::CustomMessageStart,
		ChestClosed,
		Transform,
		StateChanged,
	};

	uint16_t m_NextMessage;
	bool m_MessageAvailable;

	NetChestComp();

	uint16_t m_ChestDropValue;
	uint16_t m_State;

	void DropItem(glm::vec4 openner);
	void SetState(uint16_t state);

	//Network Component Functions
	void HandleMessage(RakNet::BitStream *bs);
	void PacketUpdate(const float& deltaTime);
	void OnUpdate(float deltaTime);
};
