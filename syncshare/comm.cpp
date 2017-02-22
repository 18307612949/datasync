#include "comm.h"
#include <unistd.h>
#include <pthread.h>
int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int* outlen)  
{  
        iconv_t cd;  
        int rc;  
        char **pin = &inbuf;  
        char **pout = &outbuf;  
        cd = iconv_open(to_charset,from_charset);  
        if (cd==0)  
                return -1;  
        memset((void*)outbuf,0, *outlen);  
        if (iconv(cd,pin,(size_t*)&inlen,pout,(size_t*)outlen) == -1)  
                return false;  
        iconv_close(cd);  
        return true;  
}  
  
bool u2g(char *inbuf,int inlen, string& strOut) {
    char *pszTemp = (char*)malloc(inlen*4);
    int outlen = inlen*4;
    bool rv = code_convert((char*)"utf-8", (char*)"gb2312", inbuf, inlen, pszTemp, &outlen);  
    free(pszTemp);
    return rv;
}  
  
bool g2u(char *inbuf,int inlen, string& strOut)  
{  
//    char *pszTemp = (char*)malloc(inlen*2)
    char pszTemp[2048];
    int outlen = 2048;
    bool rv = code_convert((char*)"gb2312", (char*)"utf-8", inbuf, inlen, pszTemp, &outlen);  
    strOut = pszTemp;
 //   free(pszTemp);
    return rv;
}  


bool gbk2utf8(string &strOut, const char *srcStr) {  
    if (NULL == srcStr) {  
        cout<< "Bad Parameter\n";  
        return false;  
    }  
  
    //首先先将utf8编码转换为unicode编码  
    if (NULL == setlocale(LC_ALL, "zh_CN.gbk")) //设置转换为unicode前的码,当前为utf8编码  
            {  
        cout<<  "Bad Parameter\n";  
        return false;  
    }  
  
    int unicodeLen = mbstowcs(NULL, srcStr, 0); //计算转换后的长度  
    if (unicodeLen <= 0) {  
        cout<< "Can not Transfer!!!\n";  
        return false;  
    }  
    wchar_t *unicodeStr = (wchar_t *) calloc(sizeof(wchar_t), unicodeLen + 1);  
    mbstowcs(unicodeStr, srcStr, strlen(srcStr)); //将utf8转换为unicode  
  
    //将unicode编码转换为gbk编码  
    if (NULL == setlocale(LC_ALL, "zh_CN.utf8")) //设置unicode转换后的码,当前为gbk  
            {  
        cout<< "Bad Parameter\n";  
        return false;  
    }  
    int outLen = wcstombs(NULL, unicodeStr, 0); //计算转换后的长度  
    char* pszTemp = (char*)calloc(sizeof(char), outLen+1);
    
    wcstombs(pszTemp, unicodeStr, outLen);  
    pszTemp[outLen] = '\0'; //添加结束符  
    strOut = pszTemp;
    free(unicodeStr);  
    free(pszTemp);
    return true;  
}  

bool utf82gbk(string &strOut, const char *srcStr) {  
    if (NULL == srcStr) {  
        cout<< "1Bad Parameter\n";
        return false;  
    }  
  
    //首先先将utf8编码转换为unicode编码  
    if (NULL == setlocale(LC_ALL, "zh_CN.utf8")) {  
        cout<< "2Bad Parameter\n";
        return false;  
    }  
  
    int unicodeLen = mbstowcs(NULL, srcStr, 0); //计算转换后的长度 
    if (unicodeLen <= 0) {  
        cout<< "3Bad Parameter\n";
        return false;  
    }  
    wchar_t *unicodeStr = (wchar_t *) calloc(sizeof(wchar_t), unicodeLen + 1);  
    mbstowcs(unicodeStr, srcStr, strlen(srcStr)); //将utf8转换为unicode  
  
    //将unicode编码转换为gbk编码  
    if (NULL == setlocale(LC_ALL, "zh_CN.gbk")) {  
        return false;  
    }  
    int outLen = wcstombs(NULL, unicodeStr, 0); //计算转换后的长度  
    char* pszTemp = (char*)calloc(sizeof(char), outLen+1);
    
    wcstombs(pszTemp, unicodeStr, outLen);  
    pszTemp[outLen] = '\0'; //添加结束符  
    strOut = pszTemp;
    free(unicodeStr);
    free(pszTemp);
    return true;  
}  

int code_convert(char *from_charset,char *to_charset,char **inbuf,size_t *inlen,char **outbuf,size_t *outlen)
{
iconv_t cd;
int rc;

cd = iconv_open(to_charset,from_charset);
if (cd==0) return -1;
memset(outbuf, 0, *outlen);
if (iconv(cd, inbuf, inlen,outbuf, outlen)==-1) return -1;
iconv_close(cd);
return 0;
}
//UNICODE码转为GB2312码
int u2g(char **inbuf,size_t *inlen,char **outbuf,size_t *outlen){
return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}
//GB2312码转为UNICODE码
int g2u(char **inbuf,size_t *inlen,char **outbuf,size_t *outlen){
return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}

vector<string> splitStrToVector(string& str){
    vector<string> result;
    int i = 0;
    int j = 0;
    while(str[i] == ' ')i++;
    j = i;
    while(i < str.length()) {
        if(str[j] == ',' || str[j] == 0) {
            result.push_back(str.substr(i, j - i));
            j++;
            i = j;
        }
        else {
            j++;
        }
    }
    return result;
}