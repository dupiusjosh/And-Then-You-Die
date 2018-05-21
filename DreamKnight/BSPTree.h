#pragma once
#include <vector>
#include "LevelContainer.h"

#define LENGTH 75
#define WIDTH 75

class BSPTree
{
public:
	struct Node
	{
		LevelContainer m_Data = LevelContainer(0, 0, WIDTH, LENGTH);
		Node * m_Left = nullptr;
		Node * m_Right = nullptr;
		Node * m_Parent = nullptr;
		Node(LevelContainer _cont) { m_Data = _cont; }
	};
	

	BSPTree();
	~BSPTree();
	void deletetree(Node* _node);
	std::vector<Node*> GetLeaves() { return m_Leaves; }
	Node* GetRoot() { return root; }
	std::vector<Node*> RandSplit(LevelContainer _container);
	void SplitContainers(LevelContainer _container, unsigned int _iter, Node* _node);
	int GetIters() { return iters; }
	int GetLength() { return m_Length; }
	int GetWidth() { return m_Width; }
private:
	Node * root;
	//int iters = 4;
	int iters = 3;
	unsigned int m_Width = WIDTH, m_Length = LENGTH;
	std::vector<Node*> m_Leaves;
};