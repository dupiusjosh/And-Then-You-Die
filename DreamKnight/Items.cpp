#include "Items.h"
#include <random>
#include "MaterialComponent.h"
#include "StatScript.h"
#include "Object.h"
#include "NetPlayerComponent.h"
#include "NetEnemyComp.h"
#include "RigidBodyComponent.h"
#include "DataManager.h"
#include "Utils.h"

void BluedThirster::OnAttackHit(Object* attacker, Object* hitObject, float& damageDealt)
{
	StatScript* stats = attacker->GetComponent<StatScript>();
	if (stats)
	{
		stats->ModifyHealth(damageDealt*healAmount);
	}
}

//Debug Item
DiscoSabre::DiscoSabre()
{
	m_Color = glm::vec4(1, 1, 1, 1);
	timer = 0.0f;
}

void DiscoSabre::OnAttackHit(Object* attacker, Object* hitObject, float& damageDealt)
{
	MaterialComponent* mat = hitObject->GetComponent<MaterialComponent>();
	if (mat)
	{
		mat->SetColor(m_Color);
	}
}

void DiscoSabre::OnAttack(Object* attacker)
{
	MaterialComponent* mat = attacker->GetComponent<MaterialComponent>();
	if (mat)
	{
		mat->SetColor(m_Color);
		//printf("Swing ColorChanged! ItemColor: (R:%f, G:%f, B:%f, A:%f)\n", m_Color.r, m_Color.g, m_Color.b, m_Color.a);
		//printf("Swing ColorChanged! MatColor: (R:%f, G:%f, B:%f, A:%f)\n", mat->m_Color.r, mat->m_Color.g, mat->m_Color.b, mat->m_Color.a);
	}
}

void DiscoSabre::OnTakeDamage(Object * hitObject, Object * attacker, float & damageTaken)
{
	RigidBodyComponent* rb = hitObject->GetComponent<RigidBodyComponent>();
	if (rb)
	{
		//rb->AddVelocity(glm::vec3(0.0f, 3.0f, 0.0f));
	}
}

void DiscoSabre::Update(Object* holder, float& deltaTime)
{
	timer += deltaTime;
	if (timer >= 1.5f)
	{
		m_Color = glm::vec4(1.0f / (rand() * 0.001f), 1.0f / (rand() * 0.001f), 1.0f / (rand() * 0.001f), 1);
		NetPlayerComponent* player = holder->GetComponent<NetPlayerComponent>();
		if (player)
		{
			auto weapon = player->GetWeapon();
			if (weapon)
			{
				MaterialComponent* mat = weapon->GetComponent<MaterialComponent>();
				if (mat)
				{
					mat->SetColor(m_Color);
				}
			}
		}
		/*else
		{
			NetEnemyComp* enemy = holder->GetComponent<NetEnemyComp>();
			if (enemy)
			{

			}
		}*/
		timer -= 1.5f;
	}
}

ChickenDinner::ChickenDinner()
{
	DataManager* dm = DataManager::GetInstance();
	size_t idx = dm->LoadMesh("Chicken.fbx");
	//Utils::PrintWarning("ChickenDinner Mesh idx: %li\n", idx);
	//size_t idx = dm->LoadMesh("Capsule.fbx");
	if (idx != LOADERROR)
	{
		m_Model = (Mesh*)dm->Get(idx, Data::DataType::MeshData);
		//Set Error Texture until proper texture is available
		//m_Model->m_Texture = 0;
	}

	//m_Color = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
}

void ChickenDinner::OnAdd(Object * holder)
{
	StatScript* stats = holder->GetComponent<StatScript>();
	if (stats)
	{
		stats->SetHealth(stats->GetMaxHealth() + HealthChange);
		stats->SetCurrentHealth(stats->GetMaxHealth());
	}
}

void ChickenDinner::OnRemove(Object * holder)
{
	StatScript* stats = holder->GetComponent<StatScript>();
	if (stats)
	{
		stats->SetMaxHealth(stats->GetMaxHealth() - HealthChange);
	}
}

Item_Base::Item_Base()
{
	DataManager* dm = DataManager::GetInstance();
	size_t idx = dm->LoadMesh("PickupCrate.fbx");
	if (idx != LOADERROR)
	{
		m_Model = (Mesh*)dm->Get(idx, Data::DataType::MeshData);
	}
}

Mesh * Item_Base::GetMesh()
{
	return m_Model;
}

glm::vec4 & Item_Base::GetColor()
{
	return m_Color;
}

AtkSpdItem::AtkSpdItem() : Item_Base()
{
	m_Color = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
}

void AtkSpdItem::OnAdd(Object * holder)
{
	StatScript* ss = holder->GetComponent<StatScript>();
	if (ss)
	{
		ss->SetAtkSpeed(ss->GetAtkSpeed() + AttackSpeedChange);
	}
}

void AtkSpdItem::OnRemove(Object * holder)
{
	StatScript* ss = holder->GetComponent<StatScript>();
	if (ss)
	{
		ss->SetAtkSpeed(ss->GetAtkSpeed() - AttackSpeedChange);
	}
}

AtkDmgItem::AtkDmgItem() : Item_Base()
{
	m_Color = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
}

void AtkDmgItem::OnAdd(Object * holder)
{
	StatScript* ss = holder->GetComponent<StatScript>();
	if (ss)
	{
		ss->SetAtkDamage(ss->GetAtkDamage() + AttackDamageChange);
	}
}

void AtkDmgItem::OnRemove(Object * holder)
{
	StatScript* ss = holder->GetComponent<StatScript>();
	if (ss)
	{
		ss->SetAtkDamage(ss->GetAtkDamage() - AttackDamageChange);
	}
}

MoveSpdItem::MoveSpdItem()
{
	m_Color = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
}

void MoveSpdItem::OnAdd(Object * holder)
{
	StatScript* ss = holder->GetComponent<StatScript>();
	if (ss)
	{
		ss->SetMoveSpeedModifier(ss->GetMoveSpeedModifier() + MoveSpeedChange);
	}
}

void MoveSpdItem::OnRemove(Object * holder)
{
	StatScript* ss = holder->GetComponent<StatScript>();
	if (ss)
	{
		ss->SetMoveSpeed(ss->GetMoveSpeedModifier() - MoveSpeedChange);
	}
}
