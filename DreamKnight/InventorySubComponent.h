#pragma once
#include "BaseNet_Comp.h"
#include "NetPlayerComponent.h"
#include "Pickup.h"
#include <vector>

#define Inventory_Size 32



class Item_Base;
class ItemRef;

class InventorySubComponent
{
public:
	struct InventoryMessage
	{
		uint16_t m_Type;
		uint16_t m_ManagerIndex;
		uint64_t m_Index;
	};

	enum class MessageTypes : uint16_t
	{
		InventoryUpdate = NetPlayerComponent::MessageTypes::InventoryUpdate,
		InventoryAdded,
		InventoryRemoved,
		numMessageTypes,
		Invalid = 65535U,
	};
private:

	std::vector<uint16_t> m_Inventory;
	unsigned int m_ItemsInInventory;
	std::vector<InventoryMessage> m_MessageBacklog;
	ItemRef* m_Reference;

public:

	

	InventorySubComponent();
	~InventorySubComponent();

	void AddItem (uint16_t pickup, bool m_isServer, Object* holder);
	void RemovePickups(bool m_isServer, Object* holder);
	bool IsFull();
	void OnUpdate(Object* holder, float& deltaTime);
	void HandleMessage(RakNet::BitStream* bs);
	//void PacketUpdate(const float& deltaTime);

	std::vector<InventoryMessage>& getMessageBacklog();

	//Event Passthrough
	void OnAttack(Object* attacker);
	void OnTakeDamage(Object* hitObject, Object* attacker, float& damageTaken);
	void OnAttackHit(Object* attacker, Object* hitObject, float& damageDealt);
	void ItemUpdate(Object* gameObject, float& deltaTime);
};



