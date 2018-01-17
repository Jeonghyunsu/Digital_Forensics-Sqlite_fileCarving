#include "stdafx.h"
#include "CarvingInfo.h"

byte * buffer;
byte* pagebuffer;
boolean deleteflag;

void SQLiteAnalysis(char filename[256], char resultPath[256], boolean _deleteflag)
{
	buffer = new byte[PAGESIZE];
	pagebuffer = new byte[PAGESIZE];
	deleteflag = _deleteflag;

	LPCTSTR fileName = (LPCTSTR)(CString)filename;
	CFile file;
	
	if (file.Open(fileName, CFile::modeRead) == FALSE) {
		printf("DB File Open fail\n");
		return ;
	}

	uint64_t nFileSize = file.GetLength();
	uint64_t seek = 0;
	uint64_t offset = 0;

	/* 
	 * <File Carving> 
     * 기존에는 파일 시스템 전체를 덤프하여 Sqlite DataBase Header 마다 테이블의 정보를 추출해내는 것을 목적으로 함
     * 테스트는 Sqlite DataBase 파일을 가지고 실험
	*/ 
	while (offset < nFileSize) {

		file.Seek(offset, CFile::begin);
		file.Read(buffer, PAGESIZE);
		for (int i = 0; i < PAGESIZE; i++) {

			//DB Header Search (of Page 1) - Sqlite DB 파일 단위로 탐색
			if (CheckPage_1(i) == true) {
				PageAnalysis(&file, offset, resultPath);
				offset += PAGESIZE - i;
				break;
			} 
			offset++;
		}
	}

	file.Close();
	delete[] buffer, pagebuffer;
}

int main(int argc, char* argv[])
{
	SQLiteAnalysis("History.db", "DB.txt", true);
	return 0;
}