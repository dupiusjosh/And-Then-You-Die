#pragma once
#include "Object.h"
#include "Component.h"
#include "MeshComponent.h"
#include "StatScript.h"

#include "NetPlayerComponent.h"
#include "InventorySubComponent.h"
#include "TimedEffectsSubComponent.h"
#include "SoundComponent.h"
#include "NetEnemyComp.h"
#include "NetEnemyRangeComp.h"
#include "NetPlayerComponent.h"


namespace HealthChangedEvents
{
	static void KillUnit(const StatScript::HealthChangedEventData* const e)
	{
		if (e->CurrHP > 0.001f)
			return;

		if (e->obj->GetComponent<NetEnemyComp>())
			e->obj->GetComponent<NetEnemyComp>()->SetState(NetEnemyComp::EnemyStates::Dead);

		if (e->obj->GetComponent<NetEnemyRangeComp>())
			e->obj->GetComponent<NetEnemyRangeComp>()->SetState(NetEnemyRangeComp::EnemyStates::Dead);

		if (e->obj->GetComponent<NetPlayerComponent>())
			e->obj->GetComponent<NetPlayerComponent>()->SetState(PlayerStates::Dead), printf("Deadzo\n");
	}

	static void DamageTaken(const StatScript::HealthChangedEventData* const e)
	{
		if (e->obj->tag == Object::ObjectTags::Enemy)
		{
			auto *a = e->obj->GetComponent<NetEnemyComp>();
			if (a)
				if(a->GetState() != NetEnemyComp::EnemyStates::Dead) //TODO Fuckfuckfuckfuckfuckfuck Fix.
					a->SetState(NetEnemyComp::EnemyStates::Stunned);
		}
	}
}

namespace DamageTakingEvents
{
	static void TriggerItems(StatScript::DamageTakingEventData* const e)
	{
		if (e->DamagedObject->tag == Object::ObjectTags::Player)
		{
			NetPlayerComponent* playerComp = e->DamagedObject->GetComponent<NetPlayerComponent>();
			SoundSourceComponent* soundSource = e->DamagedObject->GetComponent<SoundSourceComponent>();

			if (soundSource)
			{
				soundSource->SetSound(DataManager::GetInstance()->LoadSound("sounds/player_hurt.ogg"));
				soundSource->Play();
			}

			if (playerComp && playerComp->m_Inventory)
			{
				e->DamagedObject->GetComponent<NetPlayerComponent>()->m_Inventory->OnTakeDamage(e->DamagedObject, e->AttackingObject, e->valueToChangeBy);
			}
			if (playerComp && playerComp->m_TemporaryEffects)
			{
				e->DamagedObject->GetComponent<NetPlayerComponent>()->m_TemporaryEffects->OnTakeDamage(e->DamagedObject, e->AttackingObject, e->valueToChangeBy);
			}
		}
		else
		{
			if (e->DamagedObject->tag == Object::ObjectTags::Enemy)
			{
				NetEnemyComp* eComp = e->DamagedObject->GetComponent<NetEnemyComp>();
				if (eComp->m_SoundSource)
				{
					eComp->m_SoundSource->SetSound(DataManager::GetInstance()->LoadSound("sounds/enemy_hurt.ogg"));
					eComp->m_SoundSource->Play();
				}
			}
		}
	}
}

namespace StunEvents
{
	static void StunEnemy(const StatScript::StunValueChangedEventData* const e)
	{
		if (e->object->tag == Object::ObjectTags::Enemy)
		{
			if (e->CurrentStunValue > e->StunThreshold) 
			{
				//if (e->object->GetComponent<NetEnemyComp>())
					//e->object->GetComponent<NetEnemyComp>()->SetState(NetEnemyComp::EnemyStates::Stunned);

				auto *a = e->object->GetComponent<NetEnemyComp>();
				if (a)
					if (a->GetState() != NetEnemyComp::EnemyStates::Dead) //TODO Fuckfuckfuckfuckfuckfuck Fix.
						a->SetState(NetEnemyComp::EnemyStates::Stunned);
			}
		}
	}
}