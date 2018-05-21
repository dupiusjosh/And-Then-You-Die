#pragma once
#include <vector>
#include <glm\vec4.hpp>
class Object;

enum class TimedEffectType : uint16_t
{
	DiscoBuff,
	UltBuff,
	numTypes,
	INVALID = 65535,
};

//Effects are similar to items, but they can hold per-effect data and are held within the player.
struct TimedEffect
{
	TimedEffectType m_Type;
	float m_TimeRemaining;
	float m_StartTime;
	//Add any variables needed to this structure, unionise the variables that can be unioned
	float m_Damage;
	float m_Interval;
	glm::vec4 m_Vector1;

	std::vector<void(*)(TimedEffect*, Object*, float&)> m_UpdateFuncs;
	std::vector<void(*)(TimedEffect*, Object*)> m_AddFuncs;
	std::vector<void(*)(TimedEffect*, Object*)> m_RemoveFuncs;
	std::vector<void(*)(TimedEffect*, Object*)> m_AttackFuncs;
	std::vector<void(*)(TimedEffect*, Object*, Object*, float&)> m_DamagedFuncs;
	std::vector<void(*)(TimedEffect*, Object*, Object*, float&)> m_HitFuncs;

	void Update(Object* modifiedObject, float& deltaTime);
	//Use this function to add stats and other variable changes
	void OnAdd(Object* object);
	//Use this function for removing stats and other variable changes
	void OnRemove(Object* object);
	//Use this function to apply effects that should trigger on spell-cast, sword swing, or bow shot.
	void OnAttack(Object* attacker);
	//Use this function to apply effects like heal-on-hit or damage reflection.
	void OnTakeDamage(Object* hitObject, Object* attacker, float& damageTaken);
	//Use this function to modify the amount of damage to be dealt by an attack, or to apply additional on-hit effects such as burn DoT or lifesteal.
	void OnAttackHit(Object* attacker, Object* hitObject, float& damageDealt);

	void Swap(TimedEffect& other);

};

//Factory method delcaration
namespace TimedEffectFactory
{
	TimedEffect MakeDiscoBuff();
	TimedEffect MakeUltBuff();
}