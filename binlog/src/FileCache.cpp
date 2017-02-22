#include "FileCache.h"
FileCache::FileCache(){
	m_fp_offset = 0;
	m_cache = NULL;
	m_fp = NULL;
}

FileCache::~FileCache(){
	if(m_fp != NULL) {
		fclose(m_fp);
	}
}
bool FileCache::enableFp(){
	return (m_fp == NULL)?false:true;
}
bool FileCache::init(cchar *filePath) {
	if(filePath == NULL)
		return false;
	if(m_fp != NULL) {
		fclose(m_fp);
		m_fp = NULL;
	}
	m_fp = fopen(filePath, "rb");
	if(m_fp == NULL)
		return false;
	fseek(m_fp, 0, SEEK_END);
	m_file_size = ftell(m_fp);
	fseek(m_fp, 0, SEEK_SET);
	return true;
}

bool FileCache::myMalloc(size_t len) {
	myFree();
	m_cache = (uchar*)malloc(len*sizeof(uchar));
	return (m_cache == NULL)?false:true;
}
void FileCache::myFree() {
	if(m_cache != NULL) {
		free(m_cache);
		m_cache = NULL;
	}
}
bool FileCache::fileEof() {
	if(fileTell() >= m_file_size || feof(m_fp))
		return true;
	return false;
}
int FileCache::fileSeek(long loffset, int iwhere) {
	return fseek(m_fp, loffset, iwhere);
}
int FileCache::fileTell(){
	return ftell(m_fp);
}

uchar* FileCache::readCache(size_t readsize, long loffset, int iwhere, bool bfseek) {
	if(!enableFp())
		return NULL;
	m_fp_offset = loffset;
	m_fp_where = iwhere;
	
	if(bfseek) {
		if(fseek(m_fp, loffset, iwhere) != 0) {
			return NULL;
		}
	}
	if(myMalloc(readsize) == false) {
		return NULL;
	}
	size_t nRead = 0;//printf("where:%d :%d\n", ftell(m_fp), readsize);
	if((nRead = fread(m_cache, readsize, sizeof(uchar), m_fp)) != 1){
		myFree();
		return NULL;
	}
	//printf("%d, %d, %d\n", ftell(m_fp), readsize, nRead);
	return m_cache;
}

bool FileCache::readCache(uchar* buf, size_t readsize, long loffset /* = 0 */, int iwhere /* = 0 */, bool bfseek /* = false */) {
	if(!enableFp())
		return false;
	m_fp_offset = loffset;
	m_fp_where = iwhere;

	if(bfseek) {
		if(fseek(m_fp, loffset, iwhere) != 0) {
			return false;
		}
	}
	size_t nRead = 0;
	if((nRead = fread(buf, readsize, sizeof(uchar), m_fp)) != 1){
		return false;
	}
	return true;
}