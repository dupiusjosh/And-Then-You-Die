#include "LevelContainer.h"
#include <random>
#include <cmath>

LevelContainer::LevelContainer()
{
	m_Length = 50;
	m_Width = 50;
	m_X = 0;
	m_Y = 0;
	m_XCenter = (int)(m_Length / 2.0f);
	m_YCenter = (int)(m_Width / 2.0f);
}


LevelContainer::LevelContainer(int _x, int _y, int _width, int _length)
{
	m_Length = _length;
	m_Width = _width;
	m_X = _x;
	m_Y = _y;
	m_XCenter = (int)std::ceil(_width / 2.0f);
	m_YCenter = (int)std::ceil(_length / 2.0f);
}

LevelContainer::~LevelContainer()
{

}

LevelContainer LevelContainer::CreateRoom(LevelContainer _container)
{
	LevelContainer temp;

	temp.m_X = _container.m_X + (rand() % (int)floor(_container.m_Width / 3.0f));
	temp.m_Y = _container.m_Y + (rand() % (int)floor(_container.m_Length / 3.0f));
	temp.m_Width = _container.m_Width - (temp.m_X - _container.m_X);
	temp.m_Length = _container.m_Length - (temp.m_Y - _container.m_Y);
	temp.m_Width -= (rand() % (int)(temp.m_Width / 3.0f))+1;
	temp.m_Length -= (rand() % (int)(temp.m_Length / 3.0f)) + 1;
	temp.m_XCenter = (int)(temp.m_Width / 2.0f);
	temp.m_YCenter = (int)(temp.m_Length / 2.0f);
	return temp;
}

// draws from middle of 1 to 2
LevelContainer LevelContainer::CreateHall(LevelContainer _container1, LevelContainer _conatainer2)
{

	int startx = _container1.m_XCenter+_container1.m_X;
	int starty = _container1.m_YCenter+_container1.m_Y;

	int endx = _conatainer2.m_XCenter+_conatainer2.m_X;
	int endy = _conatainer2.m_YCenter+_conatainer2.m_Y;
	int w = abs(endx - startx);
	int l = abs(endy - starty);

	LevelContainer hall;
	if (w < l)
		hall = LevelContainer(startx,starty,2,l);
	else
		hall = LevelContainer(startx, starty, w, 2);

	return hall;
}