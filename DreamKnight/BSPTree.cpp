#include "BSPTree.h"
#include <stack>


BSPTree::BSPTree()
{
	Node * temp = new Node(LevelContainer(0, 0, WIDTH, LENGTH));
	root = temp;
	SplitContainers(root->m_Data, iters, root);
}

BSPTree::~BSPTree()
{
	deletetree(root);
}

void BSPTree::deletetree(Node* _node)
{
	if (_node->m_Left != nullptr)
		deletetree(_node->m_Left);
	if (_node->m_Right != nullptr)
		deletetree(_node->m_Right);
	if (_node != nullptr)
	{
		delete _node;
	}
}

std::vector<BSPTree::Node*> BSPTree::RandSplit(LevelContainer _container)
{
	LevelContainer *room1, *room2;

	if (rand() % 2 == 0) // vert split
	{
		room1 = new LevelContainer(_container.m_X, _container.m_Y, (rand() % _container.m_Width)+1, _container.m_Length);

		room2 = new LevelContainer(_container.m_X + room1->m_Width, _container.m_Y, _container.m_Width - room1->m_Width, _container.m_Length);
	
		float t1 = room1->m_Width / (float)room1->m_Length;
		float t2 = room2->m_Width / (float)room2->m_Length;
		if (t1 < room1->m_WRatio || t2 < room2->m_WRatio)
		{			
			delete room1;
			delete room2;
			return RandSplit(_container);
		}
	}
	else // horizontal split
	{
		room1 = new LevelContainer(_container.m_X, _container.m_Y, _container.m_Width, (rand() % _container.m_Length) + 1);
		room2 = new LevelContainer(_container.m_X, _container.m_Y + room1->m_Length, _container.m_Width, _container.m_Length - room1->m_Length);

		float t1 = room1->m_Length / (float)room1->m_Width;
		float t2 = room2->m_Length / (float)room2->m_Width;
		if (t1 < room1->m_LRatio || t2 < room2->m_LRatio)
		{
			delete room1;
			delete room2;
			return RandSplit(_container);
		}
	}
	std::vector<Node*> vecT;
	Node * t1 = new Node(*room1);
	vecT.push_back(t1);
	Node * t2 = new Node(*room2);
	vecT.push_back(t2);
	delete room1;
	delete room2;
	return vecT;
}

void BSPTree::SplitContainers(LevelContainer _conatainer, unsigned int _iter, Node* _node)
{
	if (_iter <= 0)
	{
		m_Leaves.push_back(_node);
	}
	if (_iter > 0)
	{
		std::vector<Node*> tmp = RandSplit(_conatainer);
		_node->m_Left = tmp[0];
		_node->m_Left->m_Parent = _node;
		SplitContainers(_node->m_Left->m_Data, _iter - 1, _node->m_Left);
		_node->m_Right = tmp[1];
		_node->m_Right->m_Parent = _node;
		SplitContainers(_node->m_Right->m_Data, _iter - 1, _node->m_Right);
	}
	
}