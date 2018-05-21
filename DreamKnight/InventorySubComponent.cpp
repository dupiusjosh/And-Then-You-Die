#include "InventorySubComponent.h"
#include "Pickup.h"
#include "Items.h"
#include "ItemRef.h"
#include "NetClient.h"
#include "NetServer.h"


InventorySubComponent::InventorySubComponent()
{
	m_Inventory.reserve(64);
	m_ItemsInInventory = 0;
	m_Reference = nullptr;
}


InventorySubComponent::~InventorySubComponent()
{
}

void InventorySubComponent::AddItem(uint16_t pickup, bool m_isServer, Object* holder)
{

	m_Inventory.push_back(pickup);
	++m_ItemsInInventory;

	/*if (m_Reference == nullptr)
	{
		m_Reference = ItemRef::GetInstance();
	}*/
	ItemRef::GetItem((ItemIDs)pickup)->OnAdd(holder);

	if (m_isServer)
	{
		InventoryMessage message;
		SecureZeroMemory(&message, sizeof(InventoryMessage));
		message.m_Type = (uint16_t)MessageTypes::InventoryAdded;
		message.m_Index = m_Inventory.size();
		message.m_ManagerIndex = pickup;
		m_MessageBacklog.push_back(message);
	}
}

void InventorySubComponent::RemovePickups(bool m_isServer, Object* holder)
{

	/*if (m_Reference == nullptr)
	{
		m_Reference = ItemRef::GetInstance();
	}*/
	for (uint16_t I : m_Inventory)
	{
		ItemRef::GetItem((ItemIDs)I)->OnRemove(holder);
	}


	m_Inventory.clear();
	m_ItemsInInventory = 0;
	if (m_isServer)
	{
		m_MessageBacklog.clear();
		InventoryMessage message;
		SecureZeroMemory(&message, sizeof(InventoryMessage));
		message.m_Type = (uint16_t)MessageTypes::InventoryRemoved;
		m_MessageBacklog.push_back(message);
	}\
}

bool InventorySubComponent::IsFull()
{
	return (m_ItemsInInventory >= Inventory_Size);
}

void InventorySubComponent::OnUpdate(Object* holder, float& deltaTime)
{
	ItemUpdate(holder, deltaTime);
}

//void InventorySubComponent::HandleMessage(RakNet::BitStream * bs)
//{
//	MessageTypes type;
//	bs->Read<MessageTypes>(type);
//	//bs->IgnoreBytes(sizeof(MessageTypes));
//	switch (type)
//	{
//	case (MessageTypes::InventoryUpdate):
//	{
//		uint64_t numMessages;
//		bs->Read<uint64_t>(numMessages);
//		InventoryMessage message;
//		for (int i = 0; i < numMessages; ++i)
//		{
//			bs->Read<uint16_t>(message.m_Type);
//			bs->Read<uint16_t>(message.m_ManagerIndex);
//			//bs->Read<uint64_t>(message.m_Index);
//
//			if ((MessageTypes)message.m_Type == MessageTypes::InventoryAdded)
//			{
//				m_Inventory.push_back(message.m_ManagerIndex);
//				m_ItemsInInventory += 1;
//			}
//			if ((MessageTypes)message.m_Type == MessageTypes::InventoryRemoved)
//			{
//				m_Inventory.clear();
//				m_ItemsInInventory = 0;
//			}
//		}
//	}
//	default:
//		break;
//	}
//}

//void InventorySubComponent::PacketUpdate(const float & deltaTime)
//{
//	if (m_isServer)
//	{
//		elpsTime += deltaTime;
//		if(elpsTime >= m_SyncRate)
//		{
//			if (m_isServer)
//			{
//				RakNet::BitStream bs;
//				bs.Write(GetNetworkID());
//				bs.Write<uint16_t>((uint16_t)MessageTypes::InventoryUpdate);
//				bs.Write<uint64_t>((uint64_t)m_MessageBacklog.size());
//				for (InventoryMessage& message : m_MessageBacklog)
//				{
//					bs.Write<uint16_t>(message.m_Type);
//					bs.Write<uint16_t>(message.m_ManagerIndex);
//					//bs.Write<uint64_t>(message.m_Index);
//				}
//				m_Server->SendObjPacket(&bs);
//			}
//			elpsTime -= m_SyncRate;
//			m_MessageBacklog.clear();
//		}
//	}
//}

std::vector<InventorySubComponent::InventoryMessage>& InventorySubComponent::getMessageBacklog()
{
	return m_MessageBacklog;
}

void InventorySubComponent::OnAttack(Object * attacker)
{/*
	if (m_Reference == nullptr)
	{
		m_Reference = ItemRef::GetInstance();
	}*/
	for (uint16_t I : m_Inventory)
	{
		ItemRef::GetItem((ItemIDs)I)->OnAttack(attacker);
	}
}

void InventorySubComponent::OnTakeDamage(Object * hitObject, Object * attacker, float& damageTaken)
{
	/*if (m_Reference == nullptr)
	{
		m_Reference = ItemRef::GetInstance();
	}*/
	for (uint16_t I : m_Inventory)
	{
		ItemRef::GetItem((ItemIDs)I)->OnTakeDamage(hitObject, attacker, damageTaken);
	}

	if (damageTaken < 1.0f)
		damageTaken = 1.0f;
}

void InventorySubComponent::OnAttackHit(Object * attacker, Object * hitObject, float& damageDealt)
{
	/*if (m_Reference == nullptr)
	{
		m_Reference = ItemRef::GetInstance();
	}*/
	for (uint16_t I : m_Inventory)
	{
		ItemRef::GetItem((ItemIDs)I)->OnAttackHit(attacker, hitObject, damageDealt);
	}
}

void InventorySubComponent::ItemUpdate(Object * gameObject, float& deltaTime)
{
	/*if (m_Reference == nullptr)
	{
		m_Reference = ItemRef::GetInstance();
	}*/
	for (uint16_t I : m_Inventory)
	{
		ItemRef::GetItem((ItemIDs)I)->Update(gameObject, deltaTime);
	}
}
