#pragma once
#include <Windows.h>
#include <wchar.h>
#include <iostream>
#include <string>
using namespace std;
class StringCoding
{
public:
	StringCoding(const string strString);
	virtual ~StringCoding(void);
	string Gbk2Utf8();
	string Utf82Gbk();
	
private:
	StringCoding(void);
	
	string m_rawString;

};

