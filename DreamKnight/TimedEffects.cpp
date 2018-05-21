#include "TimedEffects.h"
#include "MaterialComponent.h"
#include "Object.h"
#include "NetPlayerComponent.h"

//Static functions for use in timed effects
namespace EffectFuncs 
{
	static void DiscoBuff_Update(TimedEffect* me, Object* object, float& deltaTime)
	{
		MaterialComponent* mat = object->GetComponent<MaterialComponent>();
		if (mat)
			mat->SetColor(glm::vec4(1.0f / (rand()*0.001f), 1.0f / (rand()*0.001f), 1.0f / (rand()*0.001f), 1));
	}

	static void UltBuff_Attack(TimedEffect* me, Object* object)
	{
		NetPlayerComponent* npc = object->GetComponent<NetPlayerComponent>();

		if (npc)
		{
			npc->FireProjectile(npc->GetDamage() * 2.0f);
		}

	}
	static void UltBuff_Add(TimedEffect* me, Object* object)
	{
		NetPlayerComponent* npc = object->GetComponent<NetPlayerComponent>();
		if (npc)
		{
			MaterialComponent* mc = npc->GetWeapon()->GetComponent<MaterialComponent>();
			if (mc)
			{
				mc->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			}
		}
	}
	static void UltBuff_Remove(TimedEffect* me, Object* object)
	{
		NetPlayerComponent* npc = object->GetComponent<NetPlayerComponent>();
		if (npc)
		{
			MaterialComponent* mc = npc->GetWeapon()->GetComponent<MaterialComponent>();
			if (mc)
			{
				mc->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			}
		}
	}
	static void UltBuff_Update(TimedEffect* me, Object* object, float& deltaTime)
	{
		NetPlayerComponent* npc = object->GetComponent<NetPlayerComponent>();
		if (npc)
		{
			MaterialComponent* mc = npc->GetWeapon()->GetComponent<MaterialComponent>();
			if (mc)
			{
				float ratio = ((me->m_StartTime - me->m_TimeRemaining) / me->m_StartTime);
				if (ratio < 0.5f)
					mc->SetColor(glm::mix(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0, 1.0, 0.0, 1.0), ratio / 0.5f));
				else
					mc->SetColor(glm::mix(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0, 0.0, 0.0, 1.0), (ratio - 0.5f) / 0.5f));
			}
		}
	}
}

//FactoryMethods
namespace TimedEffectFactory 
{
	TimedEffect MakeDiscoBuff()
	{
		TimedEffect ret;

		ret.m_Type = TimedEffectType::DiscoBuff;
		ret.m_TimeRemaining = 3.0f;
		ret.m_StartTime = 3.0f;
		ret.m_UpdateFuncs.push_back(EffectFuncs::DiscoBuff_Update);

		return ret;
	}

	TimedEffect MakeUltBuff()
	{
		TimedEffect ret;

		ret.m_Type = TimedEffectType::UltBuff;
		ret.m_TimeRemaining = 10.0f;
		ret.m_StartTime = 10.0f;
		ret.m_AttackFuncs.push_back(EffectFuncs::UltBuff_Attack);
		ret.m_AddFuncs.push_back(EffectFuncs::UltBuff_Add);
		ret.m_RemoveFuncs.push_back(EffectFuncs::UltBuff_Remove);
		ret.m_UpdateFuncs.push_back(EffectFuncs::UltBuff_Update);

		return ret;
	}
}

//namespace std
//{
//	template<>
//	void swap<TimedEffect>(TimedEffect& A, TimedEffect&B)
//	{
//		TimedEffectType tempE = A.m_Type;
//		A.m_Type = B.m_Type;
//		B.m_Type = tempE;
//
//		float tempF = A.m_TimeRemaining;
//		A.m_TimeRemaining = B.m_TimeRemaining;
//		B.m_TimeRemaining = tempF;
//
//		tempF = A.m_StartTime;
//		A.m_StartTime = B.m_StartTime;
//		B.m_StartTime = tempF;
//
//		A.m_UpdateFuncs.swap(B.m_UpdateFuncs);
//		A.m_AddFuncs.swap(B.m_AddFuncs);
//		A.m_RemoveFuncs.swap(B.m_RemoveFuncs);
//		A.m_AttackFuncs.swap(B.m_AttackFuncs);
//		A.m_DamagedFuncs.swap(B.m_DamagedFuncs);
//		A.m_HitFuncs.swap(B.m_HitFuncs);
//	}
//}

#pragma region trigger functions
void TimedEffect::Update(Object* object, float& deltaTime)
{
	m_TimeRemaining -= deltaTime;

	if (m_UpdateFuncs.size() > 0)
	{
		for (auto& func : m_UpdateFuncs)
		{
			func(this, object, deltaTime);
		}
	}
}

void TimedEffect::OnAdd(Object * object)
{
	if (m_AddFuncs.size() > 0)
	{
		for (auto& func : m_AddFuncs)
		{
			func(this, object);
		}
	}
}

void TimedEffect::OnRemove(Object * object)
{
	if (m_RemoveFuncs.size() > 0)
	{
		for (auto& func : m_RemoveFuncs)
		{
			func(this, object);
		}
	}

	m_AddFuncs.clear();
	m_AttackFuncs.clear();
	m_DamagedFuncs.clear();
	m_HitFuncs.clear();
	m_RemoveFuncs.clear();
	m_UpdateFuncs.clear();
	m_Type = TimedEffectType::INVALID;
}

void TimedEffect::OnAttack(Object * attacker)
{
	for (auto& func : m_AttackFuncs)
	{
		func(this, attacker);
	}
}

void TimedEffect::OnTakeDamage(Object * hitObject, Object * attacker, float & damageTaken)
{
	for (auto& func : m_DamagedFuncs)
	{
		func(this, hitObject, attacker, damageTaken);
	}
}

void TimedEffect::OnAttackHit(Object * attacker, Object * hitObject, float & damageDealt)
{
	for (auto& func : m_HitFuncs)
	{
		func(this, attacker, hitObject, damageDealt);
	}
}
void TimedEffect::Swap(TimedEffect & other)
{
	TimedEffectType tempE = m_Type;
	m_Type = other.m_Type;
	other.m_Type = tempE;
	
	float tempF = m_TimeRemaining;
	m_TimeRemaining = other.m_TimeRemaining;
	other.m_TimeRemaining = tempF;
	
	tempF = m_StartTime;
	m_StartTime = other.m_StartTime;
	other.m_StartTime = tempF;
	
	m_UpdateFuncs.swap(other.m_UpdateFuncs);
	m_AddFuncs.swap(other.m_AddFuncs);
	m_RemoveFuncs.swap(other.m_RemoveFuncs);
	m_AttackFuncs.swap(other.m_AttackFuncs);
	m_DamagedFuncs.swap(other.m_DamagedFuncs);
	m_HitFuncs.swap(other.m_HitFuncs);
}
#pragma endregion