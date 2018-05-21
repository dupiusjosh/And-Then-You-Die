#pragma once

#include "..\BSPTree.h"
#include "..\LevelContainer.h"
#include "Room.h"
#include <queue>

class Room;
class ObjectManager;
struct Tile;

class LevelGenerator
{
	Room* m_Rooms;
	Tile* m_Halls;
	int m_NumRooms;
	unsigned int m_Length, m_Width;
	glm::vec4 m_ChestSpawn;
	std::vector<Tile*> m_TileGrid;
	std::vector<glm::mat4> m_EnemyPos;
	std::vector<glm::mat4> m_PlayerSpawns;
	unsigned int m_Smallest = 0, m_Largest = 0;
	
public:
	LevelGenerator(unsigned int seed, ObjectManager* objm);
	~LevelGenerator();
	std::vector<Tile*> GetTileGrid() { return m_TileGrid; }
	void SetTileGrid(std::vector<Tile*> _tile) { m_TileGrid = _tile; }
	Tile * GetTile(unsigned int _x, unsigned int _y) { if (_x <= m_Width && _y <= m_Length) return (m_TileGrid)[_x + _y*m_Length]; else return nullptr; }
	void SetTile(unsigned int _x, unsigned int _y, Tile * _tile) { if (_x <= m_Width && _y <= m_Length) (m_TileGrid)[_x + _y*m_Length] = _tile; }
	std::vector<glm::mat4> GetEnemys() { return m_EnemyPos; }
	std::vector<glm::mat4> GetPlayerSpawn() { return m_PlayerSpawns; }
	glm::vec4 GetChestSpawn() { return m_ChestSpawn; }
};

