#pragma once
#include <iostream>
#include <vector>
#include <json/json.h>
using namespace std;
class CBuildFinally
{
public:
	CBuildFinally(string &jsonData);
	~CBuildFinally(void);
	BOOL LoadJson();
	vector<string> m_sql_select;
	vector<string> m_sql_insert;
	vector<string> m_sql_update;
private:
	string &m_json_data;
	

	string JsonValueToString(Json::Value &jsonValue);
};

