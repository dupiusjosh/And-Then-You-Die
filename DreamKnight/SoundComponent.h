#pragma once
#include "glm.h"
#include <AL\al.h>
#include "AudioManager.h"
#include "Component.h"
#include "Object.h"

class SoundComponent : public Component
{
	SoundSourceComponent* source;
	bool m_Occludable;

public:

	~SoundComponent()
	{
		delete source;
	}

	SoundComponent(SoundSourceComponent* _source)
	{
		source = _source;
	}

	SoundComponent(Sound* snd)
	{
		source = new SoundSourceComponent(snd);
	}

	SoundSourceComponent* GetSource()
	{
		return source;
	}

	void OnUpdate(float deltaTime)
	{
		glm::vec4 pos = m_GameObject->m_Transform[3];
		alSource3f(source->sourceID, AL_POSITION, pos[0], pos[1], pos[2]);
		if (m_Occludable)
		{

		}
	}
};