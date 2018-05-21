#pragma once
#include <vector>
#include <map>
#include "glm.h"

struct AnimationData
{
	struct Frame
	{
		double timestampInTicks;
		double timestampInSeconds;
		std::vector<glm::mediump_fmat4x4> bonePos;
	};

	std::map<std::string, size_t> m_SpecialBones;

	unsigned int m_NumBones;
	double m_TicksPerSecond;
	double m_DurationInTicks;
	double m_DurationPerTick;
	double m_DurationInSeconds;

	std::vector<Frame> frames;
};