#include "HitManagerComponent.h"

HitManager::HitManager()
{
	HitEntities.reserve(6);
	maxHits = 1;
}

size_t HitManager::Find(Object * HitObject)
{
	if (HitEntities.size() != 0)
	{
		for (size_t i = 0, limit = HitEntities.size(); i < limit; ++i)
		{
			if (HitEntities[i].HitObject == HitObject)
			{
				return i;
			}
		}
	}
	return ~0U;
}

bool HitManager::RegisterHit(Object * HitObject)
{
	size_t index = Find(HitObject);
	if (index == ~0U)
	{
		index = HitEntities.size();
		Hit thing;
		HitEntities.push_back(thing);
	}

	if (HitEntities[index].TimesHit < maxHits)
	{
		HitEntities[index].HitObject = HitObject;
		++HitEntities[index].TimesHit;
		//printf("Entity Hit! TimesHit: %d\n", HitEntities[index].TimesHit);
		return true;
	}
	return false;
}

void HitManager::RemoveObject(Object * object)
{
	size_t index = Find(object);
	if (index != ~0U)
	{
		std::swap(HitEntities[index], HitEntities.back());
		HitEntities.pop_back();
	}
}

void HitManager::ResetHits()
{
	for (Hit& h : HitEntities)
	{
		h.TimesHit = 0;
	}

}

void HitManager::ResetObjects()
{
	HitEntities.clear();
	//printf("HitEntities Reset!\n");
}

HitManager::Hit::Hit()
{
	HitObject = nullptr, TimesHit = 0;
}
