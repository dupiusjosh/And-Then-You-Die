#ifndef _UTILS_HEADER
#define _UTILS_HEADER
#include <string>

namespace Utils
{
	void ReadFile(const char * filename, char *& Data, long & size);
	int vk2ascii(unsigned long vk, unsigned short result[3]);
	void PrintError(char* str, ...);
	void PrintWarning(char * str, ...);
	void PrintWithColor(uint16_t color, char * str, ...);

	bool isInf(float x);
	
	template<typename t>
	inline bool isInRange(t min, t max, t val)
	{
		return min <= val && max >= val;
	}

	template <typename RM, typename CM>
	void CopyMat(const RM& from, CM& to)
	{
		to[0][0] = from.a1; to[1][0] = from.a2;
		to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2;
		to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2;
		to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2;
		to[2][3] = from.d3; to[3][3] = from.d4;
	}
}
#endif