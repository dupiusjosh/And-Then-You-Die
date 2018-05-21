#pragma once
#include "ServerState.h"

#include <vector>

class Object;
class ServerGameState : public ServerState
{
	std::vector<Object*> m_Enemies;
	float elpsTime;
public:
	ServerGameState();
	void Start(NetServer * serv) override;
	void Update(float deltaTime) override;
	
};