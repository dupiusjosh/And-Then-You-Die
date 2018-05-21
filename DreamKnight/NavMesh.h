#pragma once
#include <vector>
#include <unordered_map>
#include <queue>
#include "LevelGeneration\Tile.h"
#include <chrono>

#ifndef Access2D
#define Access2D(x, y, width) (int)((y * width) + x) 
#endif
//struct Tile;



namespace Navigation
{
	struct SearchNode
	{
		float givenCost, hurCost, finCost;
		Tile * data = nullptr;
		SearchNode * parent = nullptr;
		std::vector<SearchNode*> adjecents;
	};

	static bool nodeCompare(SearchNode* const& lhs, SearchNode* const& rhs)
	{
		return (lhs->finCost) > (rhs->finCost);
	};

	struct nComp2
	{
		bool operator() (SearchNode* const& lhs, SearchNode* const& rhs)
		{
			return (lhs->finCost) > (rhs->finCost);
		}
	};

	template<typename T, typename C>
	class priority_queue : public std::priority_queue<T, std::vector<T>, C>
	{
	public:
		bool remove(const T& value)
		{
			auto it = std::find(this->c.begin(), this->c.end(), value);
			if (it != this->c.end())
			{
				this->c.erase(it);
				std::make_heap(this->c.begin(), this->c.end(), this->comp);
				return true;
			}
			else
			{
				return false;
			}
		}

		void RemoveAll()
		{
			this->c.clear();
		}

		//void myPop()
		//{
		//	std::vector<T>* temp = this->c;
		//	if (temp.size() > 1)
		//		memcpy(arr.data(), &temp[1], sizeof(temp[0]) * (temp.size() - 1));
		//	if (temp.size() > 0)
		//		temp->erase(temp->end() - 1);
		//}
	};

	class NavMesh
	{
	private:
		int _width;
		SearchNode * m_start = nullptr;
		SearchNode * m_end = nullptr;

		int m_StartRow, m_StartCol, m_EndRow, m_EndCol;

		std::unordered_map<Tile*, SearchNode*> m_SearchGraph;

		std::unordered_map<Tile*, SearchNode*> m_Visited;

		

		std::vector<Tile*> searchThis;
		std::vector<int> m_Mesh;
		
		//replace this shitty shit
		//auto cmp = [](SearchNode * lcost, SearchNode * rcost) { return lcost->finCost < rcost->finCost; };
		priority_queue<SearchNode*, nComp2> open;

		bool m_IsDone = false;
		bool m_Stopped = false;
		float timer = 0;
		std::chrono::time_point<std::chrono::system_clock> lastnow;
	public:
		std::vector<Tile*> m_returnThis;

		//NGO - Deprecated. Pull the damn thing from obj manager
		//static NavMesh * const GetInstance();
		void OnStart(std::vector<Tile*> _tiles);
		NavMesh();
		NavMesh(const NavMesh &_pass);
		~NavMesh() { Shutdown(); }
		std::vector<int> GetMesh() { return m_Mesh; }
		void SetMesh(std::vector<int> _mesh) { m_Mesh = _mesh; }
		void SetTiles(std::vector<Tile*> &_tile) { searchThis = _tile; }
		void Enter(int startRow, int startCol, int goalRow, int goalCol);
		void update(float dTime);
		void SEUpdate(int startRow, int startCol, int goalRow, int goalCol);
		void Exit();
		void Shutdown();
		bool GetDone() { return m_IsDone; }
		bool GetStopped() { return m_Stopped; }
		std::vector<Tile*> getPath();

		float distance(Tile * one, Tile * two) {return (sqrtf((float)((one->m_X - two->m_X) * (one->m_X - two->m_X) + (one->m_Y - two->m_Y) * (one->m_Y - two->m_Y))));}
	};
}