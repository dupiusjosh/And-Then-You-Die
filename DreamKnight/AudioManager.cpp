#include "AudioManager.h"
#include <assert.h>
#include "Object.h"
#include "DataManager.h"

#ifdef WIN32
	#pragma comment(lib, "libogg_x86.lib")
	#pragma comment(lib, "libvorbis_x86.lib")
	#pragma comment(lib, "libvorbisfile_x86.lib")
#else
	#pragma comment(lib, "libogg_x64.lib")
	#pragma comment(lib, "libvorbis_x64.lib")
	#pragma comment(lib, "libvorbisfile_x64.lib")
#endif
#ifdef WIN32
	#pragma comment(lib, "OpenAL32_x86.lib")
#else
	#pragma comment(lib, "OpenAL32_x64.lib")
#endif

Sound::Sound()
{
	bufferData = nullptr;
	alGenBuffers(1, &bufferID);
}

Sound::Sound(char* filename)
{
	AudioManager::GetInstance()->sounds.push_back(this);
	alGenBuffers(1, &bufferID);
	LoadOGG(filename);
}

Sound::~Sound()
{
	//AudioManager::GetInstance()->sounds.push_back(this);
	alDeleteBuffers(1, &bufferID);
}

void Sound::FreeUselessData()
{
	free(bufferData);
}

void Sound::LoadOGG(const char* fileName)
{
	int endian = 0;
	int bitStream;
	long bytes;
	char* array = (char*)malloc(BUFFER_SIZE);
	std::vector<char> buff;
	FILE *f = nullptr;

	errno_t blah = fopen_s(&f, fileName, "rb");
	Utils::PrintWarning("(%s) error: %d\n", fileName, blah);
	//assert(0 && "file not found at " && fileName);

	vorbis_info *pInfo = nullptr;
	OggVorbis_File oggFile;

	int error = ov_fopen(fileName, &oggFile);

	if (error != 0)
	{
		Utils::PrintWarning("#53 error: %d\n", error);
		return;
	}

	pInfo = ov_info(&oggFile, -1);

	format = (pInfo->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	freq = pInfo->rate;

	do {
		bytes = ov_read(&oggFile, array, BUFFER_SIZE, endian, 2, 1, &bitStream);
		buff.insert(buff.end(), array, array + bytes);
	} while (bytes > 0);

	bufferSize = (uint32_t)buff.size();
	bufferData = (char*)malloc(buff.size());

	memcpy(bufferData, buff.data(), bufferSize);

	free(array);
	ov_clear(&oggFile);

	alBufferData(bufferID, format, bufferData, bufferSize, freq);
	FreeUselessData();
}

SoundSourceComponent::SoundSourceComponent()
{
	alGenSources(1, &sourceID);
}

SoundSourceComponent::SoundSourceComponent(Sound * snd)
{
	sound = snd;
	alGenSources(1, &sourceID);
	alSourcei(sourceID, AL_BUFFER, snd->bufferID);
}

SoundSourceComponent::~SoundSourceComponent()
{
	alDeleteSources(1, &sourceID);
}

void SoundSourceComponent::SetPos(glm::vec3 v)
{
	alSource3f(sourceID, AL_POSITION, v.x, v.y, v.z);
}

void SoundSourceComponent::SetLooping(bool loop)
{
	alSourcei(sourceID, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
}

void SoundSourceComponent::SetSound(Sound * snd)
{
	sound = snd;
	alSourcei(sourceID, AL_BUFFER, snd->bufferID);
}

void SoundSourceComponent::SetSound(size_t idx)
{
	printf("sound shit: %u\n", idx);
	sound = (Sound*)DataManager::GetInstance()->Get(idx, Data::DataType::AudioData);
	alSourcei(sourceID, AL_BUFFER, sound->bufferID);
}

void SoundSourceComponent::Play()
{
	alSourcePlay(sourceID);
}

void SoundSourceComponent::Stop()
{
	alSourceStop(sourceID);
}

void SoundSourceComponent::Pause()
{
	alSourcePause(sourceID);
}

void SoundSourceComponent::OnUpdate(float deltaTime)
{
	SetPos(m_GameObject->GetWorldMatrix()[3]);
}

AudioManager::AudioManager()
{
	device = alcOpenDevice(NULL);
	assert(device && "no sound device");
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
	assert(device && "no sound context");
}

AudioManager::~AudioManager()
{
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void AudioManager::SetListenerPos(glm::vec3 pos)
{
	alListener3f(AL_POSITION, pos[0], pos[1], pos[2]);
}

void AudioManager::SetListenerOrientation(glm::vec3 look, glm::vec3 up)
{
	float values[6] = { look[0], look[1], look[2], up[0], up[1], up[2] };
	alListenerfv(AL_ORIENTATION, values);
}
