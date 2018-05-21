#include "ItemRef.h"
#include "Items.h"
#include "Utils.h"

ItemRefData ItemRef::m_Items = ItemRefData();// new Item_Base*[(uint16_t)ItemIDs::numIds];

void ItemRef::Init()
{
	m_Items = new Item_Base*[(uint16_t)ItemIDs::numIds];
	m_Items.Init();
}

Item_Base * ItemRef::GetItem(ItemIDs itemId)
{
	if (itemId < ItemIDs::numIds)
	{
		return ItemRef::m_Items[(size_t)itemId];
	}
	return nullptr;
}

void ItemRefData::clear()
{
	if (m_Items != nullptr)
	{
		for (uint16_t i = 0; i < (uint16_t)ItemIDs::numIds; ++i)
			delete m_Items[i];
		delete[] m_Items;
	}
}

ItemRefData::ItemRefData()
{
	m_Items = nullptr;
	//Utils::PrintWarning("Loading ItemRef\n");
	/*m_Items[(uint16_t)ItemIDs::ChickenDinner] = new ChickenDinner();
	m_Items[(uint16_t)ItemIDs::DiscoSabre] = new DiscoSabre();
	m_Items[(uint16_t)ItemIDs::LifeStealItem] = new BluedThirster();*/
}

ItemRefData::~ItemRefData()
{
	clear();
}

void ItemRefData::Init()
{
	m_Items[(uint16_t)ItemIDs::ChickenDinner] = new ChickenDinner();
	m_Items[(uint16_t)ItemIDs::AtkDmgItem] = new AtkDmgItem();
	m_Items[(uint16_t)ItemIDs::AtkSpdItem] = new AtkSpdItem();
	m_Items[(uint16_t)ItemIDs::MoveSpdItem] = new MoveSpdItem();
#if EnableDebugItems
	m_Items[(uint16_t)ItemIDs::DiscoSabre] = new DiscoSabre();
	m_Items[(uint16_t)ItemIDs::LifeStealItem] = new BluedThirster();
#endif //EnableDebugItems
}

ItemRefData& ItemRefData::operator=(Item_Base ** newContent)
{
	if (m_Items != newContent)
	{
		clear();
		m_Items = newContent;
	}
	return *this;
}

Item_Base*& ItemRefData::operator[](size_t index)
{
	return m_Items[index];
}
