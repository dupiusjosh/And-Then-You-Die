#pragma once
#include <glm/vec3.hpp>
#include <vector>

#define MaxHealthPickups 8

class Pickup;
class NetServer;

class PickupManager
{
	Pickup* m_HealthPickups[MaxHealthPickups];
	unsigned short m_CurrentHealthPickup;
	std::vector<Pickup*> m_ItemPickups;
	std::vector<Pickup*> m_EffectPickups;
	NetServer* m_Server;
	int m_HealthPickupValue;

public:
	PickupManager();
	~PickupManager();

	//Only Server objects should ever call this
	static PickupManager* GetInstance();
	void SetServer(NetServer* serv);
	void Init();
	void Reset();

	void SpawnHealthPickup(glm::vec3 pos);
	void SpawnItemPickup(glm::vec3 pos, uint16_t itemId);
	void SpawnEffectPickup(glm::vec3 pos, uint16_t itemId);
	void GarbageCollectPickups();
};

