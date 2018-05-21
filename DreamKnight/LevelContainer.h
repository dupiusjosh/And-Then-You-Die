#pragma once
#include <vector>

class LevelContainer 
{
private:
	std::vector<LevelContainer> halls;
public:
	int m_Width, m_Length, m_X, m_Y, m_XCenter, m_YCenter;
	int iter = 4;
	float m_LRatio = 0.45f, m_WRatio = 0.45f;
	LevelContainer();
	LevelContainer(int _x, int _y, int _width, int _length);
	~LevelContainer();
	LevelContainer CreateRoom(LevelContainer _container);
	LevelContainer CreateHall(LevelContainer _container1, LevelContainer _conatiner2);
	std::vector<LevelContainer> GetHalls() { return halls; }
};