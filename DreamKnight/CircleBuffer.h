#pragma once
#include <vector>
using namespace std;

template<typename T>
class CircleBuffer
{
	vector<T> data;
	int nextpos = 0;
	int size = 0;
public:
	CircleBuffer(uint32_t size)
	{
		data.resize(size);
	}

	~CircleBuffer()
	{
		data.resize(0);
	}

	void pushback(T item)
	{
		if (size < data.capacity())
			++size;
		data[nextpos] = item;
		if (++nextpos >= data.capacity())
			nextpos = 0;
	}

	uint32_t capacity()
	{
		return data.capacity();
	}

	uint32_t size()
	{
		return size;
	}
};

