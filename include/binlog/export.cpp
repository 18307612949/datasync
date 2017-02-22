#include <ParseLogEntry.h>
#include <tx_lock.h>
DLL_SAMPLE_API void ParseBinlogThread(const char *filePath){
	CParseLogEntry *pInstance = CParseLogEntry::getInstance();
	pInstance->refreshFileCache(filePath);
	pInstance->parseBegin();
}

DLL_SAMPLE_API void LoadBinlogDll() {
	initMutex();
}
DLL_SAMPLE_API void UnloadBinlogDll() {
	destoryMutex();
}
DLL_SAMPLE_API char* GetString() {
	return "tangxin";
}