#pragma once
#include "..\glm.h"
#include "..\Object.h"
#include "..\Mesh.h"
#include <vector>

#define RoomMaxLength	30
#define RoomMinLength	15
#define RoomMaxWidth	30
#define RoomMinWidth	15

#ifndef Access2D
#define Access2D(x, y, width) ((y * width) + x) 
#endif

class ObjectManager;
struct Tile;
struct Hall;
class Room : public Object
{
	std::vector<Tile*>* m_grid;
	std::vector<glm::mat4>* m_EnemyPos;
	std::vector<glm::mat4>* m_PlayerSpawn;
	glm::vec3 position;
	unsigned int m_Width, m_Length, m_X, m_Z, m_WidthGrid;
	bool m_Enabled, m_SpawnRoom = false, m_IsHall = false;
	int m_Value;
	int m_DoorsUnused;
	Tile** m_RoomGrid;
	std::vector<glm::vec4> m_DoorCoords;
public:
	Room();
	Room(int _w, int _l, int _x, int _z, std::vector<Tile*>* _grid, std::vector<glm::mat4>*  _enem, unsigned int m_WidthGrid, ObjectManager * objMan, bool _IsHall);
	Room(int _w, int _l, int _x, int _z, std::vector<Tile*>* _grid, std::vector<glm::mat4>* _enem, std::vector<glm::mat4>* _player, unsigned int _WidthGrid, ObjectManager * objMan, bool _isSpawnRoom);
	~Room();

	void DoMagic();
	void fillRoom(ObjectManager*);
	void SetPosition(glm::vec3&);
	unsigned int GetNumTiles();
	unsigned int GetWidth();
	unsigned int GetLength();
	void SetLength(unsigned int _len) { m_Length = _len; }
	void SetWidth(unsigned int _wid) { m_Width = _wid; }
	int getOpenDoors();
	Tile& GetTile(unsigned int i);
	glm::vec3& GetPosition();
};

