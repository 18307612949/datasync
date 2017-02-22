#ifndef _COMM_FUNC
#define _COMM_FUNC
#include <iostream>
#include <iconv.h>
#include<stdlib.h>
#include<string.h>
#include <vector>
using namespace std;
extern bool utf82gbk(string &strOut, const char *srcStr);
extern bool gbk2utf8(string &strOut, const char *srcStr);
extern char * ConvertEnc( char *encFrom, char *encTo, const char * in);

extern int u2g(char **inbuf,size_t *inlen,char **outbuf,size_t *outlen);
extern int g2u(char **inbuf,size_t *inlen,char **outbuf,size_t *outlen);
extern vector<string> splitStrToVector(string& str);
class Lock  {  
private:         
    pthread_mutex_t m_mutex;  
public:  
    Lock(pthread_mutex_t  &mutex) : m_mutex(mutex)  {  
        pthread_mutex_lock(&m_mutex);
    }  
    ~Lock()  {  
        pthread_mutex_unlock(&m_mutex);
    }  
};
#endif //_COMM_FUNC