#pragma once
#include "../Object.h"


#define MAXTILEWORTH 5
#define MINTILEWORTH -1

	class DataManager;
	class Room;

	struct Tile : public Object
	{
		enum TileType
		{
			Invalid = -1,
			None,
			Floor,
			Wall,
			Prop,
			Door,
			SlopeSide,
			SlopeCornerOuter,
			NumTypes,
		};

		enum XMLTileType
		{
			XWall = 88,
			XFloor = 71,
		};

		Room* m_ContainingRoom;
		TileType m_Type;
		size_t m_MeshIndex;
		size_t m_TextureIndex;
		unsigned int m_DataId;
		int m_Worth;
		int m_X, m_Y;
		void Update(float deltaTime);
		void RegisterContainingRoom(Room*);
		Tile();
		Tile(TileType, char*, DataManager*, Room*);
		Tile(XMLTileType _type);
		~Tile();
	};