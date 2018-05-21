#pragma once
#include <stdint.h>
#include <vector>

#define EnableDebugItems 0

enum class ItemIDs : uint16_t
{
	ChickenDinner = 0,
	AtkDmgItem,
	AtkSpdItem,
	MoveSpdItem,
#if EnableDebugItems
	DiscoSabre,
	LifeStealItem,
#endif //EnableDebugItems

	//Debug Items
	numIds,
	Invalid = (uint16_t)-1,
};

class Item_Base;

struct ItemRefData
{
	Item_Base** m_Items = nullptr;

	void clear();
	ItemRefData();
	~ItemRefData();
	void Init();
	ItemRefData& operator=(Item_Base** newContent);
	Item_Base*& operator[](size_t index);
};

class ItemRef
{
	static ItemRefData m_Items;

public:

	static void Init();
	static Item_Base* GetItem(ItemIDs itemId);
};