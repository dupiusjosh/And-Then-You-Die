#include "StatScript.h"
#include "Object.h"
#include "MeshComponent.h"
#include "StatScriptEvents.h"

void StatScript::FireHealthChanged(HealthChangedEventData eventData)
{
	for (auto iter = HealthChangedEvents.begin(); iter != HealthChangedEvents.end(); ++iter)
	{
		(*iter)(&eventData);
	}
}

void StatScript::FireHealthLost(HealthChangedEventData eventData)
{
	for (auto iter = HealthLostEvents.begin(); iter != HealthLostEvents.end(); ++iter)
	{
		(*iter)(&eventData);
	}
}

void StatScript::FireHealthGained(HealthChangedEventData eventData)
{
	for (auto iter = HealthGainedEvents.begin(); iter != HealthGainedEvents.end(); ++iter)
	{
		(*iter)(&eventData);
	}
}

void StatScript::FireStunValueChanged(StunValueChangedEventData eventData)
{
	for (auto iter = StunValueChangedEvents.begin(); iter != StunValueChangedEvents.end(); ++iter)
	{
		(*iter)(&eventData);
	}
}

void StatScript::OnUpdate(float deltaTime)
{
	m_stunValue = glm::clamp<float>(m_stunValue - 1.0f * deltaTime, 0, FLT_MAX);
}

void StatScript::AddHealthChangeEvent(void (*v)(const HealthChangedEventData* const) )
{
	HealthChangedEvents.push_back(v);
}

void StatScript::AddHealthLostEvent(void (*v)(const HealthChangedEventData* const))
{
	HealthLostEvents.push_back(v);
}

void StatScript::AddHealthGainedEvent(void (*v)(const HealthChangedEventData* const))
{
	HealthGainedEvents.push_back(v);
}

void StatScript::AddDamageTakingEvent(void(*v)(DamageTakingEventData * const))
{
	DamageTakingEvents.push_back(v);
}

void StatScript::AddStunValueChangedEvent(void(*v)(const StunValueChangedEventData * const))
{
	StunValueChangedEvents.push_back(v);
}

void StatScript::ModifyHealth(float amt)
{
	m_CurrentHealth += amt;

	if (m_CurrentHealth > m_MaxHealth)
	{
		m_CurrentHealth = m_MaxHealth;
	}
	else if (m_CurrentHealth < 0.0f)
	{
		m_CurrentHealth = 0.0f;
	}

	HealthChangedEventData e;
	e.CurrHP = m_CurrentHealth;
	e.MaxHP = m_MaxHealth;
	e.ratio = GetHealthRatio();
	e.obj = m_GameObject;
	e.valueChanged = amt;

	FireHealthChanged(e);

	if (amt > 0)
		FireHealthGained(e);
	else if (amt < 0)
		FireHealthLost(e);

}

void StatScript::TakeDamage(float damage, Object * sender)
{
	DamageTakingEventData ed;
	ed.AttackingObject = sender;
	ed.DamagedObject = m_GameObject;
	ed.CurrHP = m_CurrentHealth;
	ed.MaxHP = m_MaxHealth;
	ed.valueToChangeBy = damage;
	
	if (DamageTakingEvents.size() > 0)
	{
		for (auto& func : DamageTakingEvents)
		{
			func(&ed);
		}
	}

	ModifyHealth(-1.0f * ed.valueToChangeBy);
}

void StatScript::AddStunValue(float value)
{
	m_stunValue += value;

	StunValueChangedEventData e;
	e.CurrentStunValue = m_stunValue;
	e.StunThreshold = m_stunThreshold;
	e.object = m_GameObject;

	FireStunValueChanged(e);
}

void StatScript::SetCurrentHealth(float hp)
{
	HealthChangedEventData e;
	e.CurrHP = m_CurrentHealth;
	e.MaxHP = m_MaxHealth;
	e.ratio = GetHealthRatio();
	e.obj = m_GameObject;
	e.valueChanged = hp;

	bool lostHealth = false;

	if (hp < m_CurrentHealth)
		lostHealth = true;

	m_CurrentHealth = hp;
	glm::clamp(m_CurrentHealth, 0.0f, m_MaxHealth);

	FireHealthChanged(e);
	
	if (lostHealth)
		FireHealthLost(e);
	else
		FireHealthGained(e);
}
void StatScript::SetMaxHealth(float value)
{
	m_MaxHealth = value;
	if (m_CurrentHealth < value)
	{
		m_CurrentHealth = value;
	}
}
void StatScript::SetHealth(float value)
{
	m_MaxHealth = value;
}

void StatScript::SetAtkSpeed(float value)
{
	m_AtkSpeed = value;
}

void StatScript::SetAtkDamage(float value)
{
	m_AtkDamage = value;
}

void StatScript::SetMoveSpeed(float value)
{
	m_MoveSpeed = value;
}

void StatScript::SetMoveSpeedModifier(float value)
{
	m_MoveSpeedModifier = value;
}

void StatScript::SetStunValue(float value)
{
	m_stunValue = value;
}

void StatScript::SetStunThreshold(float value)
{
	m_stunThreshold = value;
}

void StatScript::SetStunDuration(float value)
{
	m_stunDuration = value;
}

float StatScript::GetAtkSpeed()
{
	return m_AtkSpeed;
}

float StatScript::GetAtkDamage()
{
	return m_AtkDamage;
}

float StatScript::GetMoveSpeed()
{
	return m_MoveSpeed;
}

float StatScript::GetMoveSpeedModifier()
{
	return m_MoveSpeedModifier;
}

float StatScript::GetHealthRatio()
{
	return m_CurrentHealth / m_MaxHealth;
}

float StatScript::GetStunValue()
{
	return m_stunValue;
}

float StatScript::GetStunThreshold()
{
	return m_stunThreshold;
}

float StatScript::GetStunDuration()
{
	return m_stunDuration;
}

float StatScript::GetCurrentHealth()
{
	return m_CurrentHealth;
}

float StatScript::GetMaxHealth()
{
	return m_MaxHealth;
}
