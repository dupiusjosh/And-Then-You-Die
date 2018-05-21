#pragma once
#include "glm.h"

class Object;
struct Mesh;

//Base Do-Nothing Item
class Item_Base
{
protected:
	//thing* sprite;
	Mesh* m_Model = nullptr;
	glm::vec4 m_Color = glm::vec4(1.0f,1.0f,1.0f,0.0f);

public:
	Item_Base();
	Mesh* GetMesh();
	glm::vec4& GetColor();
	//Use this to modify stats
	virtual void OnAdd(Object* holder) { return; }
	//Use this to remove stat modifications
	virtual void OnRemove(Object* holder) { return; }
	//Use this function to apply effects that should trigger on spell-cast, sword swing, or bow shot.
	virtual void OnAttack(Object* attacker) { return; }
	//Use this function to apply effects like heal-on-hit or damage reflection.
	virtual void OnTakeDamage(Object* hitObject, Object* attacker, float& damageTaken) { return; }
	//Use this function to modify the amount of damage to be dealt by an attack, or to apply additional on-hit effects such as burn DoT or lifesteal.
	virtual void OnAttackHit(Object* attacker, Object* hitObject, float& damageDealt) { /*damageDealt += 0.0f*/ return; }
	//Use this function for bonuses and regen applied over time.
	virtual void Update(Object* Holder, float& deltaTime) { /*render Sprite*/return; }
};

class ChickenDinner : public Item_Base
{
	const float HealthChange = 20.0f;
public:
	ChickenDinner();
	void OnAdd(Object* holder);
	void OnRemove(Object* holder);
}; 

class AtkSpdItem : public Item_Base
{
	const float AttackSpeedChange = 0.33f;
public:
	AtkSpdItem();
	void OnAdd(Object* holder);
	void OnRemove(Object* holder);
};

class AtkDmgItem : public Item_Base
{
	const float AttackDamageChange = 1.0f;
public:
	AtkDmgItem();
	void OnAdd(Object* holder);
	void OnRemove(Object* holder);
};

class MoveSpdItem : public Item_Base
{
	const float MoveSpeedChange = 0.4f;
public:
	MoveSpdItem();
	void OnAdd(Object* holder);
	void OnRemove(Object* holder);
};
//DebugItem
class DiscoSabre : public Item_Base
{
	//Variables that are instance-specific (Like these) should not be in items.
	glm::vec4 m_Color;
	float timer;
public:
	DiscoSabre();
	void OnAttackHit(Object* attacker, Object* hitObject, float& damageDealt);
	void Update(Object* Holder, float& deltaTime);
	void OnAttack(Object* attacker);
	void OnTakeDamage(Object* hitObject, Object* attacker, float& damageTaken);
};

class BluedThirster : public Item_Base
{
	const float healAmount = 0.3f;
public:
	void OnAttackHit(Object* attacker, Object* hitObject, float& damageDealt);
};