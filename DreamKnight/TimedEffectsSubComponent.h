#pragma once
#include "Component.h"
#include "TimedEffects.h"
#include "NetPlayerComponent.h"
#include <vector>

class TimedEffectsSubComponent
{
	enum class MessageTypes : uint16_t
	{
		EffectUpdate = NetPlayerComponent::MessageTypes::EffectsUpdate,
		EffectAdded,
		EffectsCleared,
		numMessageTypes,
		Invalid = 65535U,
	};
	std::vector<TimedEffect> m_Effects;
public:
	struct NetworkMessage
	{
		uint16_t m_MessageType;
		uint16_t m_EffectType;
	};
	
	std::vector<NetworkMessage> messageBacklog;
	std::vector<NetworkMessage>& GetMessageBacklog();

	//Removes all effects
	void ClearEffects(Object* object, bool isServer);
	//Do not use this function for looping removal. Use ClearEffects instead
	void RemoveEffect(Object* object, size_t index);
	//Adds... an effect... what did you think this did?
	void AddEffect(Object* object, TimedEffectType effect, bool isServer);

	//Updates all effects
	void OnUpdate(Object* object, float DeltaTime);
	//Use this function to apply effects that should trigger on spell-cast, sword swing, or bow shot.
	void OnAttack(Object* attacker);
	//Use this function to apply effects like heal-on-hit or damage reflection.
	void OnTakeDamage(Object* hitObject, Object* attacker, float& damageTaken);
	//Use this function to modify the amount of damage to be dealt by an attack, or to apply additional on-hit effects such as burn DoT or lifesteal.
	void OnAttackHit(Object* attacker, Object* hitObject, float& damageDealt);
};