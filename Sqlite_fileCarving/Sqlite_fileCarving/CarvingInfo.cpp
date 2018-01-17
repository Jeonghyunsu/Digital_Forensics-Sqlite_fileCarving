#include "stdafx.h"
#include "CarvingInfo.h"

unsigned int read_byte(unsigned short startN, int size)
{
	unsigned int result = 0;
	for (int i = 0; i < size; i++)
	{
		result += ((unsigned int)buffer[startN + i] << (8 * (size - 1 - i)));
	}
	return result;
}

unsigned long long page_read_byte(unsigned short startN, int size, boolean isPrint)
{
	unsigned long long result = 0;
	for (int i = 0; i < size; i++)
	{
		result += ((unsigned long long)pagebuffer[startN + i] << (8 * (size - 1 - i)));
	}

	if(isPrint == true){
		cout << result << "  ";
		printHex(startN, size);
	}
	return result;
}
void printHex(unsigned short offset, int size)
{
	printf(" (");
	for (int i = 0; i < size; i++)
	{
		printf("0x%0x, ", pagebuffer[offset + i]);
	}
	printf(")\n");
}