#ifndef INCREMENT_DATA_H
#define INCREMENT_DATA_H

#include <iostream>
#include <vector>
#include <deque>
using namespace std;
#ifdef _USRDLL
#define DLL_SAMPLE_API __declspec(dllexport)
#else
#define DLL_SAMPLE_API __declspec(dllimport)
#endif

#define TIMER_CLIENT_READ_DB_DATA 1003

typedef struct generate_data{
	string tableName_;
	vector<string> columnKey_;
	vector<string> columnVal_;
}GENERATE_DATA, *P_GENERATE_DATA;

typedef struct head_generate_data{
	string headID_;
	long headNumber_;
	long headOrder_;
	vector<P_GENERATE_DATA> headBody;
}HEAD_GENERATE_DATA, *P_HEAD_GENERATE_DATA;

extern deque<P_GENERATE_DATA> glb_queue_data;
extern deque<vector<P_HEAD_GENERATE_DATA>> glb_vec_queue_data;

extern  void StartGenerateData();
extern  void StopGenerateData();
extern  void CALLBACK TimerProc(HWND hwnd, UINT Msg, UINT idEvent, DWORD dwTime);

extern  P_GENERATE_DATA ConsumerDataGenerate();
extern  P_HEAD_GENERATE_DATA VectorHeadConsumerDataGenerate();
extern  vector<P_HEAD_GENERATE_DATA> VectorConsumerDataGenerate();

extern VOID ProductDataGenerate(P_GENERATE_DATA getDate);
extern VOID ProductDataGenerate(P_HEAD_GENERATE_DATA getDate/*P_GENERATE_DATA getDate*/);
extern VOID ProductDataGenerate(vector<P_HEAD_GENERATE_DATA> getDate);

extern bool WriteTimeUpdateXML(const string tableName,const string keyValues);

extern unsigned int GetTimeStampByStr( const char* pDate/*, int32 iNameSize*/ );
extern string GetStrFromTime(time_t iTimeStamp/*, char *pszTime*/);
//extern bool InitConfig();
//extern void readData();

#endif //INCREMENT_DATA_H