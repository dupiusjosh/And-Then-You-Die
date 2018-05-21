#include "NavMesh.h"

namespace Navigation
{
	namespace
	{
		static NavMesh * singleton = nullptr;
	}

	NavMesh::NavMesh()
	{

	}

	NavMesh::NavMesh(const NavMesh &_pass)
	{
		m_SearchGraph = _pass.m_SearchGraph;
		searchThis = _pass.searchThis;
		_width = _pass._width;
	}

	//NavMesh * const NavMesh::GetInstance()
	//{
	//	if (singleton == nullptr)
	//	{
	//		singleton = new NavMesh();
	//	}
	//	return singleton;
	//}

	bool Adjecents(Tile const* _center, Tile const* _compareThis)
	{
		//std::vector<SearchNode*> temp;
		if (_center == NULL || _compareThis == NULL || _compareThis->tag != Object::Floor)
		{
			return false;
		}
		if ((abs(_compareThis->m_X - _center->m_X) <= 1 && abs(_compareThis->m_Y - _center->m_Y) <= 1) && (_compareThis != _center))
		{
			return true;
		}
		else
		{
			return false;
		}
		//SearchNode * sertemp;
		/*for (int r = -1; r < 2; r++)
		{
			for (int c = -1; c < 2; c++)
			{
				if ((Access2D(r, c, m_width) >= 0 && Access2D(r,c,m_width) < _size) && m_Tiles[Access2D(r, c, m_width)]->tag == Object::Floor)
				{
					sertemp = new SearchNode;
					sertemp->data = m_Tiles[Access2D(r, c, m_width)];
					temp.push_back(sertemp);
				}
			}
		}*/
		return false;
	}

	void NavMesh::OnStart(std::vector<Tile*> _tiles)
	{
		searchThis = _tiles;
		int tSize = searchThis.size();
		m_Mesh.resize(tSize);
		_width = (int)(sqrt(tSize));
		for (int r = 0; r < _width; r++)
		{
			for (int c = 0; c < _width; c++)
			{
				int i = Access2D(c, r, _width);
				if (searchThis[i] != NULL && searchThis[i]->tag == Object::Floor)
				{
					SearchNode * temp;

					if (m_SearchGraph.find(searchThis[i]) == m_SearchGraph.end())
					{
						temp = new SearchNode;
						temp->data = searchThis[i];
						m_SearchGraph.insert(std::make_pair(temp->data, temp));
					}
					else
					{
						temp = m_SearchGraph[searchThis[i]];
					}

					for (int r2 = -1; r2 < 2; r2++)
					{
						for (int c2 = -1; c2 < 2; c2++)
						{
							Tile * tile;
							int some = Access2D((c + c2), (r + r2), _width);
							if (some >= 0 && some < tSize)
							{
								tile = searchThis[some];
								if (Adjecents(temp->data, tile))
								{
									if (m_SearchGraph.find(tile) != m_SearchGraph.end())
									{
										SearchNode * temp2 = m_SearchGraph[tile];
										temp->adjecents.push_back(temp2);
									}
									else
									{
										SearchNode * temp2 = new SearchNode;
										temp2->data = tile;
										m_SearchGraph.insert(std::make_pair(tile, temp2));
										temp->adjecents.push_back(temp2);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	void NavMesh::Enter(int startRow, int startCol, int goalRow, int goalCol)
	{
		if (startRow >= _width || startCol >= _width || goalCol >= _width || goalRow >= _width
			|| startRow < 0 || startCol < 0 || goalCol < 0 || goalRow < 0)
		{
			return;
		}
		Tile* start = searchThis[Access2D(startRow, startCol, _width)];
		if (start == nullptr)
			return;
		m_start = m_SearchGraph[start];

		Tile* goal = searchThis[Access2D(goalRow, goalCol, _width)];
		if (goal == nullptr)
			return;
		m_end = m_SearchGraph[goal];

		if (m_start == NULL || m_end == NULL)
		{
			return;
		}
		else
		{
			m_start->givenCost = 0;
			m_start->hurCost = distance(m_start->data, m_end->data);
			m_start->finCost = m_start->givenCost + m_start->hurCost * 1.2f;
			m_start->parent = nullptr;
			m_end->givenCost = distance(m_start->data, m_end->data);

			open.push(m_start);

			m_Visited[m_start->data] = m_start;
		}
	}
void NavMesh::update(float dTime)
	{
		while (!open.empty())
		{
			//if (dTime == 0)
			//	break;

			SearchNode * current = open.top();
			open.pop();

			if (current == m_end)
			{
				m_IsDone = true;
				break;
			}

			for (size_t i = 0; i < current->adjecents.size(); i++)
			{
				SearchNode * neh = current->adjecents[i];
				float tempgCost = current->givenCost + 1 * distance(current->data, neh->data);

				if (m_Visited[neh->data]/*iter*/ != nullptr)
				{
					//auto iterr = std::find(visited.begin(), visited.end(), current);
					if (tempgCost < m_Visited[neh->data]->givenCost /*(*iter)->cost*/)
					{

						SearchNode * someone = m_Visited[neh->data];// (*iter);
						//open.pop();
						open.remove(someone);
						someone->parent = current;

						someone->givenCost = tempgCost;
						someone->finCost = someone->givenCost + (someone->hurCost*1.2f);

						open.push(someone);
					}
				}
				else
				{
					SearchNode * someone;
					someone = neh;
					someone->parent = current;

					someone->givenCost = tempgCost;
					someone->hurCost = distance(neh->data, m_end->data);
					someone->finCost = someone->givenCost + someone->hurCost*1.2f;
					m_Visited[neh->data] = someone;

					open.push(someone);
				}
			}
		}
	}

	void NavMesh::SEUpdate(int startRow, int startCol, int goalRow, int goalCol)
	{
		if (startRow >= _width || startCol >= _width || goalCol >= _width || goalRow >= _width
			|| startRow < 0 || startCol < 0 || goalCol < 0 || goalRow < 0)
		{
			return;
		}
		Exit();
		if (searchThis[Access2D(startRow, startCol, _width)] == nullptr || searchThis[Access2D(goalRow, goalCol, _width)] == nullptr)
		{
			return;
		}
		m_start = m_SearchGraph[searchThis[Access2D(startRow, startCol, _width)]];
		m_end = m_SearchGraph[searchThis[Access2D(goalRow, goalCol, _width)]];
		if (m_start == NULL || m_end == NULL)
		{
			return;
		}
		else
		{
			m_start->givenCost = 0;
			m_start->hurCost = distance(m_start->data, m_end->data);
			m_start->finCost = m_start->givenCost + m_start->hurCost * 1.2f;
			m_start->parent = nullptr;
			m_end->givenCost = distance(m_start->data, m_end->data);

			open.push(m_start);

			m_Visited[m_start->data] = m_start;
		}
	}

	void NavMesh::Exit()
	{
		
		//while (!open.empty())
		//{
		//	open.remove(open.top());
		//}
		open.RemoveAll();
		m_Visited.clear();
	}
	void NavMesh::Shutdown()
	{
		Exit();
		if (searchThis.size() != 0 && m_SearchGraph.size() != 0)
		{
			for (unsigned int i = 0; i < searchThis.size(); i++)
			{
				Tile* tmp = searchThis[i];
				if (searchThis[i] != NULL && m_SearchGraph[tmp])
					delete m_SearchGraph[tmp];
			}
			m_SearchGraph.clear();
		}
		//printf("nav stuff %d", m_SearchGraph.size());
	}

	std::vector<Tile*> NavMesh::getPath()
	{
		//std::vector<Tile*> temp;
		m_returnThis.clear();
		SearchNode * curr = m_end;
		int count = 0;
		while ((curr != nullptr || curr != NULL) && m_end->data->tag == Object::Floor && curr->data->tag == Object::Floor)
		{
			m_returnThis.push_back(curr->data);
			curr = curr->parent;
			count++;
			if (count > 30)
				break;
		}
		//printf("Nav count : %d\n", count);
		//printf("\nPath : %d", count);
		return m_returnThis;
	}

}