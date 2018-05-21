#pragma once
#include "Component.h"
#include "AnimationData.h"

#define NotAnimating 0i8
#define LoopAnimating 1i8
#define SingleAnimating 2i8
#define OneFrameAnimating 3i8

struct Mesh;

class AnimationComponent : public Component
{
	Mesh* m_Mesh;
	AnimationData* m_AnimData;
	
	size_t m_CurrFrame;//, m_NextFrame;
	
	char m_State;

	float m_Speed;
	glm::mediump_fmat4x4* m_InterpolatedMatrices; 
	unsigned int m_NumInterpolatedMatrices;
	float m_CurrFrameTimeDelta;
	float m_CurrTime;

	AnimationData::Frame * m_prevFrame = nullptr, *m_nextFrame = nullptr;


	bool debug;
	std::vector<Object*> m_DebugJointObjects;

public:
	AnimationComponent(Mesh*);
	~AnimationComponent();

	void OnUpdate(float deltaTime);
	void SetAnimationState(char state);
	void SetDebug(bool active);
	//Return Value: 0 if success, -1 on fail
	int SetAnimation(const char* animName, float exitTime = .2f);
	void SetAnimationSpeed(float SpeedMultiplier);

	unsigned int getNumInterpolatedMatricies() const;
	unsigned int GetMatrixIndexByName(const char * name);
	const glm::mediump_fmat4x4* getInterpolatedMatrices() const;
	const glm::mediump_fmat4x4* const GetInterpolatedMatrixByName(const char * name);
	const glm::mediump_fmat4x4* const GetInterpolatedMatrixByID(unsigned int i);
	float GetCurrentTotalAniDuration();
};

