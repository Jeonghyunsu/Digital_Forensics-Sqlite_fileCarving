#include "stdafx.h"
#include "Page1_Analyzer.h"
#include <locale.h>

using namespace std;

FILE* wfile;

void recursionAnalyzer(unsigned int PageNum, CFile* file, uint64_t offset, boolean isSchema, DB_header* db);
void SchemaAnalyzer(node_header* page1_header, uint64_t offset, unsigned short page1_cell_offset, CFile* file, DB_header* db, unsigned previous_offset);

void DB_HeaderRead(DB_header* db, unsigned short curr_offset)
{
	db->pagesize = page_read_byte(curr_offset + 16, 2, false);
	db->database_size = page_read_byte(curr_offset + 28, 4, false);
	db->freePage_offset = page_read_byte(curr_offset + 32, 4, false);
	db->freePageNumber = page_read_byte(curr_offset + 36, 4, false);
	db->IncrementalVacuumSettings = page_read_byte(curr_offset + 52, 4, false);
	db->textEncoding = page_read_byte(curr_offset + 56, 4, false);
	db->incremental_vacuum_mode = page_read_byte(curr_offset + 64, 4, false);
}

void PageAnalysis(CFile* file, uint64_t offset, char resultPath[256])
{
	// 테이블 정보를 저장할 파일 Open
	wfile = fopen(resultPath, "w");
	
	//DB Header Read [PAGESIZE]
	file->Seek(offset, CFile::begin);
	file->Read(pagebuffer, PAGESIZE);

	DB_header *db = new DB_header();
	unsigned short curr_offset = 0;
	
	// DB header //
	db->header_offset = offset;
	DB_HeaderRead(db, curr_offset);

	// Page 1 header
	node_header* page1_header = new node_header();


	// 1. 삭제되지 않은 Record 복구 // 
	byte pageheader = page_read_byte(curr_offset + 100, 1, false);
	unsigned short page1_cell_offset;
	
	// Page 1이 Leaf Node인 경우 바로 Schema Table을 분석
	if (pageheader == TABLE_BTREE_LEAF_NODE) {
		page1_header->pageflag = pageheader;

		page1_header->first_unalocated_Blk_offset = page_read_byte(curr_offset + 101, 2, false); //offset of first block fo free space
		page1_header->cell_count = page_read_byte(curr_offset + 103, 2, false); // Number of record
		page1_header->first_cell_offset = page_read_byte(curr_offset + 105, 2, false); // Offset of the first bytes of the record
		page1_header->over_3byte_unallocated_BlkCnt = page_read_byte(curr_offset + 107, 1, false); // Num of fragemented free bytes
		page1_cell_offset = curr_offset + 108;


		SchemaAnalyzer(page1_header, offset, page1_cell_offset, file, db, PAGESIZE);
	}
	// Page 1이 Internal Node인 경우 Page 1의 cell이 가리키는 Page에서 Shcema Table을 분석
	else if (pageheader == TABLE_BTREE_INTERNAL_NODE) {
		page1_header->pageflag = pageheader;

		page1_header->first_unalocated_Blk_offset = page_read_byte(curr_offset + 101, 2, false); //offset of first block fo free space
		page1_header->cell_count = page_read_byte(curr_offset + 103, 2, false); // Number of record
		page1_header->first_cell_offset = page_read_byte(curr_offset + 105, 2, false); // Offset of the first bytes of the record
		page1_header->over_3byte_unallocated_BlkCnt = page_read_byte(curr_offset + 107, 1, false); // Num of fragemented free bytes
		page1_header->Page_number_of_right_most_child_page = page_read_byte(curr_offset + 108, 4, false); // Internale Page only : Page number of right most child-page

		page1_cell_offset = curr_offset + 112;

		boolean Page_number_of_right_most_child_page_flag = true;
		int cell_count = 0;
		unsigned int cell_offset;
		while (cell_count < page1_header->cell_count)
		{
				file->Seek(offset, CFile::begin);
				file->Read(pagebuffer, PAGESIZE);

				cell_offset = page_read_byte(page1_cell_offset, 2, false);
			
				if (cell_offset < PAGESIZE && cell_offset != 0) {

				unsigned int child_page_num = page_read_byte(cell_offset, 4, false);
				unsigned int varint = page_read_byte(cell_offset + 4, 1, false);

				// Recursion을 통해 Node에 접근
				if (child_page_num <= db->database_size) recursionAnalyzer(child_page_num, file, db->header_offset, true, db);
				if (child_page_num == page1_header->Page_number_of_right_most_child_page)Page_number_of_right_most_child_page_flag = false;
			}
			page1_cell_offset += 2;
			cell_count++;
		}

		// 남은 정보가 존재할 수 있는 가장 오른쪽에 존재하는 Child Page를 Recursion을 통해 Node에 접근
		if (Page_number_of_right_most_child_page_flag == true)
		{
			recursionAnalyzer(page1_header->Page_number_of_right_most_child_page, file, db->header_offset, true, db);
		}
	}

	fprintf(wfile, "L*A*S*T");
	fclose(wfile);
}

void recursionAnalyzer(unsigned int PageNum, CFile* file, uint64_t offset, boolean isSchema, DB_header* db)
{
	file->Seek(offset + CalPageOffset(PageNum), CFile::begin);
	file->Read(pagebuffer, PAGESIZE);
	
	node_header* page_header = new node_header();
	byte pageheader = page_read_byte(0, 1, false);
	unsigned short page_cell_offset;
	
	if ( pageheader == TABLE_BTREE_LEAF_NODE) {
			
		page_header->pageflag = pageheader;

		page_header->first_unalocated_Blk_offset = page_read_byte(1, 2, false); //offset of first block fo free space
		page_header->cell_count = page_read_byte(3, 2, false); // Number of record
		page_header->first_cell_offset = page_read_byte(5, 2, false); // Offset of the first bytes of the record
		page_header->over_3byte_unallocated_BlkCnt = page_read_byte(7, 1, false); // Num of fragemented free bytes
		
		page_cell_offset = 8;

		int cell_count = 0;
		unsigned int cell_offset;
		unsigned int previous_offset = PAGESIZE;
		
		while (cell_count < page_header->cell_count)
		{
			file->Seek(offset + CalPageOffset(PageNum), CFile::begin);
			file->Read(pagebuffer, PAGESIZE);

			
			cell_offset = page_read_byte(page_cell_offset, 2, false);
			if (cell_offset < PAGESIZE && cell_offset != 0) {

				if (isSchema == true) {
					//Schema type인 경우 Schema 정보를 Carving
					boolean check1 = false;
					unsigned short root_page_num = 0;
					unsigned int curr_offset = cell_offset;

					
					while (curr_offset < previous_offset)
					{		
						if (!check1 && CheckShcemaType(curr_offset) == true) {
							check1 = true;
						}
						if (check1 && (root_page_num = CheckShcemaTable(curr_offset)))
						{
							//root page Info
							char query[PAGESIZE] = { '\0' };
							int queryCnt = 0;
							int leftCnt = 0;
							for (int i = curr_offset; i < previous_offset; i++)
							{
								query[queryCnt++] += pagebuffer[i];

								if (pagebuffer[i] == '(')leftCnt++;
								if (pagebuffer[i] == ')') {
									leftCnt--;
									if (leftCnt == 0)break;
								}
							}
						
							string queryStr = queryCheck(query);
							cout << queryStr << endl << endl;
						
							fprintf(wfile, "%s\n", queryStr.c_str());

							check1 = false;
						
							if (cell_offset < PAGESIZE && cell_offset != 0) {					
								recursionAnalyzer(pagebuffer[root_page_num], file, offset, false, db);
							}

						}
						
						curr_offset++;
					}
					previous_offset = cell_offset;
					

				}
				else {
					// Leaf Node // Data OverFlow는 다루지 않았음. 
					LeafAnalyzer(pageheader, cell_offset, PageNum);									
				}
			}
			page_cell_offset += 2;
			cell_count++;
		}	

		boolean _deleteflag = deleteflag;
		while (_deleteflag)
		{
			file->Seek(offset + CalPageOffset(PageNum), CFile::begin);
			file->Read(pagebuffer, PAGESIZE);

			cell_offset = page_read_byte(page_cell_offset, 2, true);
			if (cell_offset < PAGESIZE && cell_offset != 0) {
				if (isSchema == false ) {
					if (PageNum <= db->database_size) {
						LeafAnalyzer(pageheader, cell_offset, PageNum);
					}
				}
			}
			else _deleteflag = false;

			page_cell_offset += 2;		
		}
	}
	else if (pageheader == TABLE_BTREE_INTERNAL_NODE) {

		page_header->pageflag = pageheader;

		page_header->first_unalocated_Blk_offset = page_read_byte(1, 2, false); //offset of first block fo free space
		page_header->cell_count = page_read_byte(3, 2, false); // Number of record
		page_header->first_cell_offset = page_read_byte(5, 2, false); // Offset of the first bytes of the record
		page_header->over_3byte_unallocated_BlkCnt = page_read_byte(7, 1, false); // Num of fragemented free bytes
		page_header->Page_number_of_right_most_child_page = page_read_byte(8, 4, false); // Internale Page only : Page number of right most child-page
		
		page_cell_offset = 12;

		boolean Page_number_of_right_most_child_page_flag = true;

		int cell_count = 0;
		unsigned int cell_offset;
		while (cell_count < page_header->cell_count)
		{
			file->Seek(offset + CalPageOffset(PageNum), CFile::begin);
			file->Read(pagebuffer, PAGESIZE);

			cell_offset = page_read_byte(page_cell_offset, 2, false);
			if (cell_offset < PAGESIZE && cell_offset != 0) {

				unsigned int child_page_num = page_read_byte(cell_offset, 4, false);
				unsigned int varint = page_read_byte(cell_offset + 4, 1, false);

				if (child_page_num <= db->database_size)
					recursionAnalyzer(child_page_num, file, offset, isSchema, db);

				if (child_page_num == page_header->Page_number_of_right_most_child_page)Page_number_of_right_most_child_page_flag = false;
			}
			page_cell_offset += 2;
			cell_count++;
		}	

		if (Page_number_of_right_most_child_page_flag == true)
		{
			recursionAnalyzer(page_header->Page_number_of_right_most_child_page, file, offset, isSchema, db);
		}

	}
}

void LeafAnalyzer(byte pageheader, unsigned int cell_offset, int PageNum)
{
	// 실제 저장되어 있는 Row Data를 추출해내는 과정
	// C#에서 Encoding Mode에 따라 Decoding할 것이기 때문에 Hex 단위로 파일 쓰기를 함 
	// fprintf에서 이해할 수 없는 문자들은 추출한 Row Data들의 단위를 구별하기 위함
	if (pageheader == TABLE_BTREE_LEAF_NODE)
	{		
		int start_offset = cell_offset;
		int getByteNum;
		int length_first_header=0;

		int length_of_record = ParsingResult(cell_offset, &getByteNum);
		cell_offset += getByteNum; length_first_header += getByteNum;

		int rowID = ParsingResult(cell_offset, &getByteNum);
		cell_offset += getByteNum; length_first_header += getByteNum;

		int length_of_data_header = ParsingResult(cell_offset, &getByteNum);
		int record_data_start_offset = cell_offset + length_of_data_header;
		cell_offset += getByteNum;

		int left_data_header = length_of_data_header - getByteNum;

		int cnt = 0;
		int rowCnt = 0;
		while (cnt < left_data_header && record_data_start_offset <= start_offset + length_first_header+ length_of_record)
		{
			
			int dataheader = ParsingResult(cell_offset, &getByteNum);
			int datasize;

			if (dataheader == 0) {
				datasize = 0;
				fprintf(wfile, "NULL\n", datasize);
			}
			else if (dataheader >= 1 && dataheader <= 4) {
				datasize = dataheader;
				fprintf(wfile, "%d\n", (int)page_read_byte(record_data_start_offset, datasize, false), datasize);
			}
			else if (dataheader == 5) {
				datasize = 6;
				fprintf(wfile, "%llu\n", (unsigned long long)page_read_byte(record_data_start_offset, datasize, false), datasize);
			}
			else if (dataheader == 6) {
				datasize = 8;
				fprintf(wfile, "%llu\n", (unsigned long long)page_read_byte(record_data_start_offset, datasize, false), datasize);
			}
			else if (dataheader == 7) {
				datasize = 8;
				fprintf(wfile, "%llu\n", (unsigned long long)page_read_byte(record_data_start_offset, datasize, false), datasize);
			}
			else if (dataheader == 8) {
				datasize = 0;
				fprintf(wfile, "0\n", datasize);
			}
			else if (dataheader == 9) {
				datasize = 0;
				fprintf(wfile, "1\n", datasize);
			}
			else if (dataheader == 10 || dataheader == 11) {
				datasize = 0; 
				fprintf(wfile, "Not Used\n", datasize);
			}
			else if (dataheader >= 12 && dataheader % 2 == 0) {
				datasize = (dataheader - 12) / 2;
				fprintf(wfile, "BLOB\n", datasize);
			}
			else if (dataheader >= 13 && dataheader % 2 == 1) {
				datasize = (dataheader - 13) / 2;

				if(datasize!=0)fprintf(wfile, "^^^\n");
				else fprintf(wfile, "$$$\n");

				byte parsing[PAGESIZE] = {0};
				int parsingCnt = 0;
				for (int i = record_data_start_offset; i < record_data_start_offset + datasize; i++)
				{
					if (i <= PAGESIZE) {
						parsing[parsingCnt++] = (byte)pagebuffer[i];
						fprintf(wfile, "%0x ", parsing[parsingCnt-1]);
					}
					else break;
				}
				
				if(datasize == 0)fprintf(wfile, "NULL\n");
				else fprintf(wfile, "\n");
			}
			
			cell_offset += getByteNum;
			cnt += getByteNum;
			record_data_start_offset += datasize;	
			rowCnt++;
		}
		if (rowCnt != 0)fprintf(wfile, "*****\n");
	}	
}

void SchemaAnalyzer(node_header* page1_header, uint64_t offset, unsigned short page1_cell_offset, CFile* file, DB_header* db, unsigned previous_offset)
{
	int cell_count = 0;
	unsigned int cell_offset;

	while (cell_count < page1_header->cell_count)
	{
		file->Seek(offset, CFile::begin);
		file->Read(pagebuffer, PAGESIZE);


		cell_offset = page_read_byte(page1_cell_offset, 2, false);
		if (cell_offset < PAGESIZE && cell_offset != 0) {

			//Schema type
			boolean check1 = false;
			unsigned short root_page_num = 0;
			unsigned int curr_offset = cell_offset;


			while (curr_offset < previous_offset)
			{

				if (!check1 && CheckShcemaType(curr_offset) == true) {
					check1 = true;
				}
				if (check1 && (root_page_num = CheckShcemaTable(curr_offset)))
				{
					//root page Info
					char query[PAGESIZE] = {'\0'};
					int queryCnt = 0;
					int leftCnt = 0;
					for (int i = curr_offset; i < previous_offset; i++)
					{
						query[queryCnt++] += pagebuffer[i];

						if (pagebuffer[i] == '(')leftCnt++;
						if (pagebuffer[i] == ')') {
							leftCnt--;
							if (leftCnt == 0)break;
						}
					}
			
					string queryStr = queryCheck(query);
					cout << queryStr << endl<<endl;

					check1 = false;
			
					fprintf(wfile, "%s\n", queryStr.c_str());

					if (cell_offset < PAGESIZE && cell_offset != 0) {
						recursionAnalyzer(pagebuffer[root_page_num], file, offset, false, db);
					}
				}

				curr_offset++;
			}
			previous_offset = cell_offset;
		}
		page1_cell_offset += 2;
		cell_count++;
	}
}


boolean is_0left(byte one_read)
{
	if (one_read >> 7 == 0)return true;
	return false;
}
int GetVariant(unsigned int cell_offset)
{
	int offsetCnt = 1;
	while (offsetCnt < 8)
	{
		byte one_read = (byte)page_read_byte(cell_offset + offsetCnt - 1, 1, false);
		if (is_0left(one_read) == true) {
			return offsetCnt;
		}
		offsetCnt++;
	}
	return 8;
}
int ParsingResult(unsigned int cell_offset, int* getByteNum)
{
	*getByteNum = GetVariant(cell_offset);
	
	int cnt = 0;
	int shift = 6;
	int result = 0;

	while (cnt < *getByteNum)
	{
		byte one_read = (byte)page_read_byte(cell_offset + *getByteNum - 1 - cnt, 1, false);

		if (cnt == 0 && *getByteNum == 8) shift = 7;
		else shift = 6;

		for (int i = 0; i <= shift; i++)
		{
			byte cal = (one_read >> i);
			if (cal % 2 == 1)
			{
				result += 1 << ((shift + 1) * cnt + i);
			}
		}
		cnt++;
	}
	
	return result;
}

unsigned int CalPageOffset(int pagenum)
{
	return (pagenum - 1)*PAGESIZE;
}

boolean CheckPage_1(int i)
{
	if (buffer[i] == sqlite_header[0] &&
		buffer[i + 1] == sqlite_header[1] &&
		buffer[i + 2] == sqlite_header[2] &&
		buffer[i + 3] == sqlite_header[3] &&
		buffer[i + 4] == sqlite_header[4] &&
		buffer[i + 5] == sqlite_header[5] &&
		buffer[i + 6] == sqlite_header[6] &&
		buffer[i + 7] == sqlite_header[7] &&
		buffer[i + 8] == sqlite_header[8] &&
		buffer[i + 9] == sqlite_header[9] &&
		buffer[i + 10] == sqlite_header[10] &&
		buffer[i + 11] == sqlite_header[11] &&
		buffer[i + 12] == sqlite_header[12] &&
		buffer[i + 13] == sqlite_header[13] &&
		buffer[i + 14] == sqlite_header[14] &&
		buffer[i + 15] == sqlite_header[15]) {
		return true;
	}
	else {
		return false;
	}
}
boolean CheckShcemaType(int i)
{
	if (pagebuffer[i] == schema_type[0] &&
		pagebuffer[i + 1] == schema_type[1] &&
		pagebuffer[i + 2] == schema_type[2] &&
		pagebuffer[i + 3] == schema_type[3] &&
		pagebuffer[i + 4] == schema_type[4]
		) {
		return true;
	}
	else {
		return false;
	}
}
unsigned short CheckShcemaTable(int i)
{
	if (pagebuffer[i] == schema_CREATE_TABLE[0] &&
		pagebuffer[i + 1] == schema_CREATE_TABLE[1] &&
		pagebuffer[i + 2] == schema_CREATE_TABLE[2] &&
		pagebuffer[i + 3] == schema_CREATE_TABLE[3] &&
		pagebuffer[i + 4] == schema_CREATE_TABLE[4] &&
		pagebuffer[i + 5] == schema_CREATE_TABLE[5] &&
		pagebuffer[i + 6] == schema_CREATE_TABLE[6] &&
		pagebuffer[i + 7] == schema_CREATE_TABLE[7] &&
		pagebuffer[i + 8] == schema_CREATE_TABLE[8] &&
		pagebuffer[i + 9] == schema_CREATE_TABLE[9] &&
		pagebuffer[i + 10] == schema_CREATE_TABLE[10] &&
		pagebuffer[i + 11] == schema_CREATE_TABLE[11]
		) {
		return i - 1;
	}
	else {
		return 0;
	}
}

string queryCheck(char query[4096])
{
	string queryStr(query);
	int offset = queryStr.length() - 1;

	if (queryStr[offset] != ')')
	{
		while (queryStr[offset] != ')' && offset >= 0)
		{
			offset--;
		}
		string queryresult = queryStr.substr(0, offset+1);
		return queryresult;
	}
	else return queryStr;
}