#pragma once
#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>
#include <stdint.h>
#include <malloc.h>
#include <vector>
#include "glm.h"
#define BUFFER_SIZE 32768

struct Sound
{
	ALenum format;
	ALsizei freq;
	char* bufferData;
	uint32_t bufferSize;
	ALuint bufferID;

	Sound();
	Sound(char* filename);
	~Sound();
	void FreeUselessData();
	void LoadOGG(const char* fileName);
};

#include "Component.h"

struct SoundSourceComponent : public Component
{
	ALuint sourceID;
	Sound* sound;
	SoundSourceComponent();
	SoundSourceComponent(Sound* sound);
	~SoundSourceComponent();
	void SetPos(glm::vec3 v);
	void SetLooping(bool loop);
	void SetSound(Sound* snd);
	void SetSound(size_t idx);
	void Play();
	void Stop();
	void Pause();

	void OnUpdate(float deltaTime);
};

class AudioManager
{
	ALCdevice* device;
	ALCcontext* context;

public:
	std::vector<Sound*> sounds;

	static AudioManager* GetInstance()
	{
		static AudioManager* obj = new AudioManager();
		return obj;
	}
	AudioManager();
	~AudioManager();

	void SetListenerPos(glm::vec3 pos);
	void SetListenerOrientation(glm::vec3 look, glm::vec3 up);
};