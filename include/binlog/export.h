#ifndef EXPORT_H
#define EXPORT_H
#ifdef _USRDLL
#define DLL_SAMPLE_API __declspec(dllexport)
#else
#define DLL_SAMPLE_API __declspec(dllimport)
#endif
extern DLL_SAMPLE_API void LoadBinlogDll();
extern DLL_SAMPLE_API void UnloadBinlogDll();
extern DLL_SAMPLE_API void ParseBinlogThread(const char *filePath);
extern DLL_SAMPLE_API char* GetString();
#endif //EXPORT_H