#pragma once
#include "Object.h"

class NetEnemy :
	public Object
{
private:
	
public:
	NetEnemy();
	virtual ~NetEnemy();
	//void UpdatePlayers(std::vector<glm::mat4> _trans) { m_playerTransforms = _trans; }
};

