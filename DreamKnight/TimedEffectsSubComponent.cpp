#include "TimedEffectsSubComponent.h"
#include "TimedEffects.h"

std::vector<TimedEffectsSubComponent::NetworkMessage>& TimedEffectsSubComponent::GetMessageBacklog()
{
	return messageBacklog;
}

void TimedEffectsSubComponent::ClearEffects(Object* object, bool isServer)
{
		for (TimedEffect& e : m_Effects)
		{
			e.OnRemove(object);
		}
		m_Effects.clear();
		if (isServer)
		{
			NetworkMessage message;
			message.m_MessageType = (uint16_t)MessageTypes::EffectsCleared;
		}
}

void TimedEffectsSubComponent::RemoveEffect(Object* object, size_t index)
{
	m_Effects[index].OnRemove(object);
	//std::swap(m_Effects[index], m_Effects.back());
	m_Effects[index].Swap(m_Effects.back());
	m_Effects.pop_back();
}

void TimedEffectsSubComponent::OnUpdate(Object* object, float DeltaTime)
{
	for (TimedEffect& E : m_Effects)
	{
		E.Update(object, DeltaTime);
	}

	for (size_t i = 0; i < m_Effects.size(); ++i)
	{
		if (m_Effects[i].m_TimeRemaining <= 0.0f)
		{
			RemoveEffect(object, i);
			--i;
		}
	}
}

void TimedEffectsSubComponent::OnAttack(Object * attacker)
{
	for (TimedEffect& E : m_Effects)
	{
		E.OnAttack(attacker);
	}
}

void TimedEffectsSubComponent::OnTakeDamage(Object * hitObject, Object * attacker, float & damageTaken)
{
	for (TimedEffect& E : m_Effects)
	{
		E.OnTakeDamage(hitObject, attacker, damageTaken);
	}
	if (damageTaken < 1.0f)
		damageTaken = 1.0f;
}

void TimedEffectsSubComponent::OnAttackHit(Object * attacker, Object * hitObject, float & damageDealt)
{
	for (TimedEffect& E : m_Effects)
	{
		E.OnAttackHit(attacker, hitObject, damageDealt);
	}
}

void TimedEffectsSubComponent::AddEffect(Object* object, TimedEffectType effect, bool isServer)
{
	if (isServer)
	{
		NetworkMessage ret;
		ret.m_MessageType = (uint16_t)MessageTypes::EffectAdded;
		ret.m_EffectType = (uint16_t)effect;
		messageBacklog.push_back(ret);
	}

	for (auto& e : m_Effects)
	{
		if (e.m_Type == effect)
		{
			e.m_TimeRemaining = e.m_StartTime;
			return;
		}
	}

	switch (effect)
	{
	case TimedEffectType::DiscoBuff: {
		m_Effects.push_back(TimedEffectFactory::MakeDiscoBuff());
		m_Effects.back().OnAdd(object);
		break;
	case TimedEffectType::UltBuff: {
		m_Effects.push_back(TimedEffectFactory::MakeUltBuff());
		m_Effects.back().OnAdd(object);
	}
	}
	}

}
