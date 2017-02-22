#pragma once
#include <iostream>
#include <vector>
using namespace std;
#ifdef _USRDLL
#define DLL_SAMPLE_API __declspec(dllexport)
#else
#define DLL_SAMPLE_API __declspec(dllimport)
#endif

typedef struct generate_data{
	string tableName_;
	vector<string> columnKey_;
	vector<string> columnVal_;
}GENERATE_DATA, *P_GENERATE_DATA;

extern DLL_SAMPLE_API void StartGenerateData();
extern DLL_SAMPLE_API void StopGenerateData();
extern DLL_SAMPLE_API P_GENERATE_DATA ConsumerGenerateData();
extern 