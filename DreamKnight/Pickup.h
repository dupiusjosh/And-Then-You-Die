#pragma once
#include "object.h"
#include <glm/mat4x4.hpp>
class PickupManager;
struct PickupNetworkComponent;
struct MaterialComponent;

enum class PickupType
{
	Invalid = -1,
	HealthPickup,
	ItemPickup,
	EffectPickup,

	numTypes,
};

class Pickup : public Object
{
	friend class PickupManager;
	PickupType m_Type;

	//You see what ActuallyAddObjects() is making me do?
	bool m_Activated;

public:
	PickupNetworkComponent* m_Controller;
	union {
		uint16_t m_ItemIndex;
		uint16_t m_EffectId;
		uint16_t m_HealthValue;
	}m_Value;
	bool m_Collected;

	void Start();
	void Update(float deltaTime) override;

	Pickup();
	~Pickup();

	void Init(char* path, PickupType type);

	void Activate(glm::mat4 transform, uint16_t value);
	void Deactivate();

	void SetMesh(uint16_t ItemId);
	void Swap(Pickup* other);
};

