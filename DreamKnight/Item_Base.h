
class Object;

//Base Do-Nothing Item
struct Item_Base
{
	void OnAttack(Object* attacker) { return; }
	void OnTakeDamage(Object* hitObject, Object* attacker, float damageTaken) { return; }
	void OnAttackHit(Object* attacker, Object* hitObject, float damageDealt) { return; }
	void Update(Object* Holder, float deltaTime) { return; }
};