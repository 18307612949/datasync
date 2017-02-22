/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CDownloadBinlog.cpp
 * Author: tx
 * 
 * Created on 2016年4月13日, 下午8:12
 */

#include "CDownloadBinlog.h"
#include <stdlib.h>
#include <time.h>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <string.h>
#include<algorithm>
#include "urlencode.h"
#include "base64.h"

CDownloadBinlog::CDownloadBinlog() {
}

CDownloadBinlog::CDownloadBinlog(const CDownloadBinlog& orig) {
}

CDownloadBinlog::~CDownloadBinlog() {
}
/*
string CDownloadBinlog::getFormatTime() {
    char szTime[128] = {0};
    time_t curTime = time(NULL);
    strftime(szTime, 128 ,"%04Y-%02m-%02dT%02H:%02M:%02SZ",localtime(&curTime));
    return szTime;
}
string CDownloadBinlog::getRandom() {
    time_t curTime = time(NULL);
    srand(curTime);
    int iRand = rand() % 1000;
    char szBuf[256] = {0};
    sprintf(szBuf, "%d%04d", curTime, iRand);
    return szBuf;
}
void CDownloadBinlog::buildParams() {
    m_params.push_back(API_PARAM("AccessKeyId", API_AccessKeyId));
    m_params.push_back(API_PARAM("Action", API_Action));
    m_params.push_back(API_PARAM("DBInstanceId", DB_InstanceId));
    m_params.push_back(API_PARAM("EndTime", "2016-04-14T11:00:00Z"));
    m_params.push_back(API_PARAM("Format", "XML"));
    m_params.push_back(API_PARAM("PageSize", "30"));
    m_params.push_back(API_PARAM("PageNumber", "1"));
    m_params.push_back(API_PARAM("SignatureMethod", API_SignatureMethod));
    m_params.push_back(API_PARAM("SignatureNonce", getRandom()));
    m_params.push_back(API_PARAM("SignatureVersion", API_SignatureVersion));
    m_params.push_back(API_PARAM("StartTime", "2016-04-14T09:00:00Z"));
    m_params.push_back(API_PARAM("Timestamp", getFormatTime()));
    m_params.push_back(API_PARAM("Version", API_Version));
}

string CDownloadBinlog::buildStrToSign() {
    sort(m_params.begin(), m_params.end(), less<API_PARAM>());//greater
    int nSize = m_params.size();
    string strSign = "";
    for(int i = 0; i < nSize - 1; i++) {
        strSign += m_params[i].key_;
        strSign += "=";
        /*
        if(m_params[i].key_.compare("Timestamp") == 0) {
            int iRv;
            char *p  = url_encode(m_params[i].value_.c_str(), m_params[i].value_.length(), &iRv);
            strSign += string(p);
            free(p);
            
        }
        else {
            strSign += m_params[i].value_;
        }
        strSign += m_params[i].value_;
        strSign += "&";
    }
    strSign += m_params[nSize - 1].key_;
    strSign += "=";
    strSign += m_params[nSize - 1].value_;
    return strSign;
}

string CDownloadBinlog::buildStrToRequest() {
    int nSize = m_params.size();
    string str = "";
    for(int i = 0; i < nSize - 1; i++) {
        str += m_params[i].key_;
        str += "=";
        str += m_params[i].value_;
        str += "&";
    }
    str += m_params[nSize - 1].key_;
    str += "=";
    str += m_params[nSize - 1].value_;
    return str;
}

size_t curlCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    cout<< (char*)contents<< endl;
}
void CDownloadBinlog::getDownloadList() {
    buildParams();
    string strSign = buildStrToSign();cout<< strSign.c_str()<< endl;
    int i;
    strSign = string("POST&%2F&") + url_encode(strSign.c_str(), strSign.length(), &i);cout<< strSign.c_str()<< endl;
    unsigned char *szHmac;
    unsigned int rLen;
    hmacEncode("sha1", API_SecretKey, strlen(API_SecretKey), strSign.c_str(), strSign.length(), szHmac, rLen);
    string base64Sign = base64_encode(szHmac, rLen);
    free(szHmac);
    m_params.push_back(API_PARAM("Signature", base64Sign));
    string strRequest = buildStrToRequest();
    cout<< strRequest.c_str()<< endl;
    
 //   string strToSign = "GET&%2F&AccessKeyId%3Dtestid%26Action%3DDescribeDBInstances%26Format%3DXML%26RegionId%3Dregion1%26SignatureMethod%3DHMAC-SHA1%26SignatureNonce%3DNwDAxvLU6tFE0DVb%26SignatureVersion%3D1.0%26TimeStamp%3D2013-06-01T10%253A33%253A56Z%26Version%3D2014-08-15";
 //   unsigned char *p;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strRequest.c_str());    // 指定post内容
        curl_easy_setopt(curl, CURLOPT_URL, API_REQUEST_URL);   // 指定url
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallback);
 //       curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
          //  char *ct;
        //    memset(ct, 0x00, 1024);
      //      res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct); 
        //    cout<< ct<< endl<< strlen(ct);
        }
        
        curl_easy_cleanup(curl);
    }
    else {
        cout<< "curl_easy_init failed"<< endl;
    }
}

int CDownloadBinlog::hmacEncode(const char * algo,  
                const char * key, unsigned int key_length,  
                const char * input, unsigned int input_length,  
                unsigned char * &output, unsigned int &output_length) {  
        const EVP_MD * engine = NULL;  
        if(strcasecmp("sha512", algo) == 0) {  
                engine = EVP_sha512();  
        }  
        else if(strcasecmp("sha256", algo) == 0) {  
                engine = EVP_sha256();  
        }  
        else if(strcasecmp("sha1", algo) == 0) {  
                engine = EVP_sha1();  
        }  
        else if(strcasecmp("md5", algo) == 0) {  
                engine = EVP_md5();  
        }  
        else if(strcasecmp("sha224", algo) == 0) {  
                engine = EVP_sha224();  
        }  
        else if(strcasecmp("sha384", algo) == 0) {  
                engine = EVP_sha384();  
        }  
        else if(strcasecmp("sha", algo) == 0) {  
                engine = EVP_sha();  
        }  
        //       else if(strcasecmp("md2", algo) == 0) {               engine = EVP_md2();        } 
        else {  
                return -1;  
        }  
  
        output = (unsigned char*)malloc(EVP_MAX_MD_SIZE);  
        HMAC_CTX ctx;  
        HMAC_CTX_init(&ctx);  
        HMAC_Init_ex(&ctx, key, strlen(key), engine, NULL);  
        HMAC_Update(&ctx, (unsigned char*)input, strlen(input));        // input is OK; &input is WRONG !!!  
  
        HMAC_Final(&ctx, output, &output_length);  
        HMAC_CTX_cleanup(&ctx);  
  
        return 0;  
}  */