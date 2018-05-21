#pragma once
#include <list>
#include "BaseNet_Comp.h"

class StatScript : public Component
{
public:

	struct HealthChangedEventData
	{
		float CurrHP;
		float MaxHP;
		float ratio;
		float valueChanged;
		Object* obj;
	};

	struct StunValueChangedEventData
	{
		float CurrentStunValue;
		float StunThreshold;
		Object* object;
	};

	struct DamageTakingEventData
	{
		float valueToChangeBy;
		float CurrHP;
		float MaxHP;
		Object* AttackingObject;
		Object* DamagedObject;
	};

private:
	float m_CurrentHealth;
	float m_MaxHealth;
	float m_AtkSpeed;
	float m_AtkDamage;
	float m_MoveSpeed;
	float m_MoveSpeedModifier = 1.0f;
	float m_stunValue;
	float m_stunThreshold = 100.0f;
	float m_stunDuration;

	std::list<void(*)(const HealthChangedEventData* const)> HealthChangedEvents;
	std::list<void(*)(const HealthChangedEventData* const)> HealthGainedEvents;
	std::list<void(*)(const HealthChangedEventData* const)> HealthLostEvents;
	std::list<void(*)(DamageTakingEventData* const)> DamageTakingEvents;
	std::list<void(*)(const StunValueChangedEventData* const)> StunValueChangedEvents;

	void FireHealthChanged(HealthChangedEventData eventData);
	void FireHealthLost(HealthChangedEventData eventData);
	void FireHealthGained(HealthChangedEventData eventData);
	void FireStunValueChanged(StunValueChangedEventData eventData);

public:

	StatScript(float currentHealth = 100.0f, float maxHealth = 100.0f, float atkSpeed = 1.0f, float atkDamage = 10.0f, float moveSpeed = 10.0f)
	{
		m_CurrentHealth = currentHealth;
		m_MaxHealth = maxHealth;
		m_AtkSpeed = atkSpeed;
		m_AtkDamage = atkDamage;
		m_MoveSpeed = moveSpeed;
	};

	void OnUpdate(float deltaTime);

	void AddHealthChangeEvent(void(*v) (const HealthChangedEventData * const));
	void AddHealthLostEvent(void(*v) (const HealthChangedEventData * const));
	void AddHealthGainedEvent(void(*v) (const HealthChangedEventData * const));
	void AddDamageTakingEvent(void(*v) (DamageTakingEventData * const));
	void AddStunValueChangedEvent(void(*v) (const StunValueChangedEventData * const));
	
	void ModifyHealth(float);
	//Torn between TakeDamage, DealDamage, and ApplyDamage for this function's name
	void TakeDamage(float, Object* sender);

	void AddStunValue(float value);

	void SetHealth(float);
	void SetCurrentHealth(float hp);
	void SetMaxHealth(float);
	void SetAtkSpeed(float);
	void SetAtkDamage(float);
	void SetMoveSpeed(float);
	void SetMoveSpeedModifier(float value);
	void SetStunValue(float value);
	void SetStunThreshold(float value);
	void SetStunDuration(float value);

	float GetAtkSpeed();
	float GetAtkDamage();
	float GetMoveSpeed();
	float GetMoveSpeedModifier();
	float GetCurrentHealth();
	float GetMaxHealth();
	float GetHealthRatio();
	float GetStunValue();
	float GetStunThreshold();
	float GetStunDuration();
};