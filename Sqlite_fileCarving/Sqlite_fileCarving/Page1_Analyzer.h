#pragma once
#include "afx.h"
#include <string>
#include <atlstr.h>
#include "CarvingInfo.h"
#include <stdio.h>
#include <ctime>

using namespace std;

void PageAnalysis(CFile* file, uint64_t offset, char resultPath[256]);
void LeafAnalyzer(byte pageheader, unsigned int cell_offset, int PageNum);

unsigned int CalPageOffset(int pagenum);
boolean CheckPage_1(int i);
boolean CheckShcemaType(int i);
unsigned short CheckShcemaTable(int i);

boolean is_0left(byte one_read);
int GetVariant(unsigned int cell_offset);
int ParsingResult(unsigned int cell_offset, int* getByteNum);

string queryCheck(char query[4096]);