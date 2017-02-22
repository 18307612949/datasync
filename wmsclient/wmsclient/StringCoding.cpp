#include "StdAfx.h"
#include "StringCoding.h"


StringCoding::StringCoding(void)
{
}

StringCoding::StringCoding(const string strString){
	m_rawString = strString;
}


StringCoding::~StringCoding(void)
{
}

string StringCoding::Gbk2Utf8(){
	string strOutUTF8 = "";  
    WCHAR * str1;  
    int n = MultiByteToWideChar(CP_ACP, 0, m_rawString.c_str(), -1, NULL, 0);  
    str1 = new WCHAR[n];  
    MultiByteToWideChar(CP_ACP, 0, m_rawString.c_str(), -1, str1, n);  
    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);  
    char * str2 = new char[n];  
    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);  
    strOutUTF8 = str2;  
    delete[]str1;  
    str1 = NULL;  
    delete[]str2;  
    str2 = NULL;  
    return strOutUTF8;  
}

string StringCoding::Utf82Gbk() {
	int len = MultiByteToWideChar(CP_UTF8, 0, m_rawString.c_str(), -1, NULL, 0);  
    unsigned short * wszGBK = new unsigned short[len + 1];  
    memset(wszGBK, 0, len * 2 + 2);  
    MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)m_rawString.c_str(), -1, (LPWSTR)wszGBK, len);  
  
    len = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL);  
    char *szGBK = new char[len + 1];  
    memset(szGBK, 0, len + 1);  
    WideCharToMultiByte(CP_ACP,0, (LPWSTR)wszGBK, -1, szGBK, len, NULL, NULL);   
    std::string strTemp(szGBK);  
    delete[]szGBK;  
    delete[]wszGBK;  
    return strTemp;  
}