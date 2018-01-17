#pragma once

#include <afx.h>
#include "windows.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <queue>
#include "Page1_Analyzer.h"
#include <time.h>
#include <sstream>
#include <algorithm>

using namespace std;

#define PAGESIZE 4096

extern byte * buffer;
extern byte* pagebuffer;
extern boolean deleteflag;

static const byte sqlite_header[16] = { 0x53, 0x51, 0x4c, 0x69, 0x74, 0x65, 0x20, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x20, 0x33, 0x00 };
static const byte schema_type[5] = { 0x74, 0x61, 0x62, 0x6C, 0x65 };
static const byte schema_CREATE_TABLE[12] = { 0x43, 0x52, 0x45, 0x41, 0x54, 0x45, 0x20, 0x54, 0x41, 0x42, 0x4C, 0x45 };

enum BTreeIden : byte {
	INDEX_BTREE_INTERNAL_NODE = 0x02,
	INDEX_BTREE_LEAF_NODE = 0x0A,
	TABLE_BTREE_INTERNAL_NODE = 0x05,
	TABLE_BTREE_LEAF_NODE = 0x0D
};

struct DB_header
{
	uint64_t header_offset;
	uint64_t search_offset;
	unsigned short pagesize;
	unsigned int database_size;
	unsigned int freePage_offset;
	unsigned int freePageNumber;
	unsigned int IncrementalVacuumSettings;
	unsigned int textEncoding;
	unsigned int incremental_vacuum_mode;
};

struct node_header
{
	byte pageflag;
	unsigned int first_unalocated_Blk_offset;
	unsigned int cell_count;
	unsigned int first_cell_offset;
	unsigned int over_3byte_unallocated_BlkCnt;
	unsigned int Page_number_of_right_most_child_page;
};

unsigned int read_byte(unsigned short startN, int size);
unsigned long long  page_read_byte(unsigned short startN, int size, boolean isPrint);
void printHex(unsigned short offset, int size);

// C# 개발 환경에서 DataBase 파일 정보를 출력할 것이기 때문에 동적 라이브러리 메소드로 생성
extern "C" __declspec(dllexport) void SQLiteAnalysis(char filename[256], char resultPath[256], boolean _deleteflag);