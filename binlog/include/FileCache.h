#pragma once
#include <cstdio>
#include <cstdlib>
#include "tx_common.h"
class FileCache{
public:
	FileCache();
	~FileCache();
	bool init(cchar *filePath);
	bool enableFp();
	uchar* readCache(size_t readsize, long loffset = 0, int iwhere = 0, bool bfseek = false);
	bool readCache(uchar* buf, size_t readsize, long loffset = 0, int iwhere = 0, bool bfseek = false);
	int fileSeek(long loffset, int iwhere);
	int fileTell();
	bool fileEof();
	uint32 getFileSize(){return m_file_size;}

private:
	FILE *m_fp;
	long m_fp_offset;
	int m_fp_where;
	uchar *m_cache;

	ulonglong m_file_size;
	bool myMalloc(size_t len);
	void myFree();
};