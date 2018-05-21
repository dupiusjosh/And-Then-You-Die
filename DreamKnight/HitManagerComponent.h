#pragma once
#include "Component.h"
#include <vector>

//Place this on weapon or attack objects
struct HitManager : public Component
{
	struct Hit
	{
		Object* HitObject;
		int TimesHit;

		Hit();
	};

	std::vector<Hit> HitEntities;
	int maxHits;

	HitManager();
	//If The referenced Object exists in the vector, returns it's index. Otherwise returns ~0U
	size_t Find(Object* HitObject);
	//Increments number of hits tracked on an object by 1. Returns false if number of hits would exceed max number of hits.
	bool RegisterHit(Object* HitObject);
	//Removes the reference to a single object, in case the object dies or something
	void RemoveObject(Object* object);
	//Leaves references to all objects, but sets all hit counts to 0. O(n)
	void ResetHits();
	//Removes all references to objects. Vector must be refilles after calling this function. O(1)
	void ResetObjects();

	void OnUpdate(float detaTime) {return;}
};