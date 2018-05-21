#pragma once
#include "ServerState.h"
class ServerLobbyState : public ServerState
{

public:
	ServerLobbyState();
	void Start(NetServer* serv) override;
	void Update(float deltaTime) override;
};