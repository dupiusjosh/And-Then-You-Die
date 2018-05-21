#include "AnimationComponent.h"
#include "MeshComponent.h"
#include "Object.h"
#include "Mesh.h"
#include "glm.h"

AnimationComponent::AnimationComponent(Mesh* mesh)
{
	debug = false;
	m_Mesh = mesh;
	m_AnimData = mesh->m_Animations.begin()->second;
	m_InterpolatedMatrices = new glm::mediump_fmat4x4[m_AnimData->m_NumBones];
	m_NumInterpolatedMatrices = m_AnimData->m_NumBones;
	m_Speed = 1.0f;
	memcpy(m_InterpolatedMatrices, m_AnimData->frames[0].bonePos.data(), sizeof(glm::mat4) * m_NumInterpolatedMatrices);
	
	m_CurrTime = 0.0;
	m_CurrFrame = 0;
	//m_NextFrame = 1;
	m_State = NotAnimating;
}

AnimationComponent::~AnimationComponent()
{
	SetDebug(false);
	delete[] m_InterpolatedMatrices;
}

void AnimationComponent::OnUpdate(float deltaTime)
{
	if (debug)
	{
		for (unsigned int boneIndex = 0; boneIndex < m_NumInterpolatedMatrices; ++boneIndex)
		{
			m_DebugJointObjects[boneIndex]->m_Transform = m_InterpolatedMatrices[boneIndex] * glm::mediump_fmat4x4(m_GameObject->m_Transform);
		}
		deltaTime = 0.05f;
	}

	if (m_State != NotAnimating)
	{
		float ratio = 0;
		m_CurrTime += (deltaTime*m_Speed);

		while (m_CurrTime > m_CurrFrameTimeDelta)
		{
			//Increment the frame.
			m_prevFrame = m_nextFrame;
			m_CurrFrame++;

			if (m_CurrFrame == m_AnimData->frames.size())
			{
				if (m_State == LoopAnimating)
				{
					m_CurrFrame = 1;
					m_nextFrame = &m_AnimData->frames[1];
					//m_CurrFrameTimeDelta = (float)m_AnimData->frames[1].timestampInSeconds;
					//m_CurrTime -= m_CurrFrameTimeDelta;
					m_CurrTime -= (float)m_AnimData->frames[1].timestampInSeconds;
					m_CurrFrameTimeDelta = (float)m_nextFrame->timestampInSeconds;
				}
				else
				{
					m_State = NotAnimating;
					break;
				}
			}
			else
			{
				m_nextFrame = &m_AnimData->frames[m_CurrFrame];
				m_CurrFrameTimeDelta = (float)(m_AnimData->frames[m_CurrFrame].timestampInSeconds - m_AnimData->frames[m_CurrFrame - 1].timestampInSeconds);
				m_CurrTime -= m_CurrFrameTimeDelta;
				m_CurrFrameTimeDelta = (float)(m_nextFrame->timestampInSeconds - m_prevFrame->timestampInSeconds);
			}

			if (m_State == OneFrameAnimating)
			{
				m_State = NotAnimating;
			}

		}

		ratio = m_CurrTime / m_CurrFrameTimeDelta;

		if (ratio > 1.0f) { ratio = 1.0f; }

		for (unsigned int boneIndex = 0; boneIndex < m_AnimData->m_NumBones; ++boneIndex)
		{
			m_InterpolatedMatrices[boneIndex] = glm::interpolate((m_prevFrame->bonePos[boneIndex]), (m_nextFrame->bonePos[boneIndex]), ratio);
		}

		glm::mediump_fmat4x4 rotation;
		glm::rotate(rotation, 180.0f, glm::mediump_fvec3(0, 1, 0));
		for (unsigned int i = 0; i < m_NumInterpolatedMatrices; ++i)
			m_InterpolatedMatrices[i] = rotation * m_InterpolatedMatrices[i];
	}
	//else
	//{
	//	printf("Animation: NotAnimating\n");
	//}
}

void AnimationComponent::SetAnimationState(char state)
{
	m_State = state;
}

void AnimationComponent::SetDebug(bool active)
{
	if (debug == active)
		return;

	debug = active;

	ObjectManager* om = ObjectManager::GetInstance();

	if (active == true && !m_DebugJointObjects.size())
	{
		m_DebugJointObjects.resize(m_NumInterpolatedMatrices);
		auto dm = DataManager::GetInstance();
		auto index = dm->LoadMesh("Sphere.fbx", true);

		for (Object*& p : m_DebugJointObjects)
		{
			p = new Object();
			p->AddComponent(new MeshComponent((Mesh*)dm->Get(index)));
			om->AddObject(p);
		}
	}
}

int AnimationComponent::SetAnimation(const char * animName, float exitTime)
{
	if (m_Mesh->m_Animations.find(animName) == m_Mesh->m_Animations.end())
	{
		return -1;
	}

	m_AnimData = m_Mesh->m_Animations[animName];

	//This is holden code, really shouldn't be needed, as animations should have same bone count per rig....
	if (m_NumInterpolatedMatrices < m_AnimData->m_NumBones)
	{
		delete[] m_InterpolatedMatrices;
		m_InterpolatedMatrices = new glm::mediump_fmat4x4[m_AnimData->m_NumBones];
	}
	m_CurrFrame = 1;
	m_nextFrame = &m_AnimData->frames[1];
	m_CurrFrameTimeDelta = (float)m_nextFrame->timestampInSeconds + exitTime;
	if (!m_prevFrame)
	{
		m_prevFrame = m_nextFrame;
		m_nextFrame = &m_AnimData->frames[2];
		m_CurrFrameTimeDelta = (float)(m_nextFrame->timestampInSeconds - m_prevFrame->timestampInSeconds);
	m_CurrFrame = 2;
	}
	m_CurrTime = 0;

	return 0;
}

void AnimationComponent::SetAnimationSpeed(float SpeedMultiplier)
{
	m_Speed = SpeedMultiplier;
}

const glm::mediump_fmat4x4 * AnimationComponent::getInterpolatedMatrices() const
{
	return m_InterpolatedMatrices;
}

unsigned int AnimationComponent::getNumInterpolatedMatricies() const
{
	return m_NumInterpolatedMatrices;
}

unsigned int AnimationComponent::GetMatrixIndexByName(const char * name)
{
	unsigned int res = -1;
	if (m_Mesh->m_BoneNameToIndex.find(name) != m_Mesh->m_BoneNameToIndex.end())
	{
		res = m_Mesh->m_BoneNameToIndex[name];
		//res = &m_InterpolatedMatrices[i];
	}
	else
	{
		if (m_AnimData->m_SpecialBones.find(name) != m_AnimData->m_SpecialBones.end())
		{
			res = m_AnimData->m_SpecialBones[name];
			//res = &m_InterpolatedMatrices[i];
		}
	}

	return res;
}
const glm::mediump_fmat4x4 * const AnimationComponent::GetInterpolatedMatrixByName(const char * name)
{
	glm::mediump_fmat4x4 * res = NULL;
	if (m_Mesh->m_BoneNameToIndex.find(name) != m_Mesh->m_BoneNameToIndex.end())
	{
		unsigned int i = m_Mesh->m_BoneNameToIndex[name];
		res = &m_InterpolatedMatrices[i];
	}
	else
	{
		if (m_AnimData->m_SpecialBones.find(name) != m_AnimData->m_SpecialBones.end())
		{
			unsigned int i = m_AnimData->m_SpecialBones[name];
			res = &m_InterpolatedMatrices[i];
		}
	}

	return res;
}

const glm::mediump_fmat4x4 * const AnimationComponent::GetInterpolatedMatrixByID(unsigned int i)
{
	if (i < m_NumInterpolatedMatrices)
	{
		return &m_InterpolatedMatrices[i];
	}
	return nullptr;
}

float AnimationComponent::GetCurrentTotalAniDuration()
{
	return (float)m_AnimData->m_DurationInSeconds;
}
