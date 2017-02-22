/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CDownloadBinlog.h
 * Author: tx
 *
 * Created on 2016年4月13日, 下午8:12
 */

#ifndef CDOWNLOADBINLOG_H
#define CDOWNLOADBINLOG_H
#include <iostream>
#include <vector>
using namespace std;

#define API_REQUEST_URL "http://rds.aliyuncs.com"
#define API_Version "2014-08-15"
#define API_AccessKeyId  "0iUkPdUiWcgSsw26"
#define API_SecretKey      "85w2EneWAUHZsHVcQBUGztuKl2SA7E"
#define API_SignatureMethod "HMAC-SHA1"
#define API_SignatureVersion "1.0"
#define API_Action "DescribeBinlogFiles"
#define DB_InstanceId "rds96m286dva3kvk4507"
typedef struct api_param{
    string key_;
    string value_;
    api_param(string key, string value):key_(key),value_(value){}
    bool   operator <  (const   api_param&   rhs   )  const {
        return this->key_.compare(rhs.key_) < 0?true:false;
    }
    bool operator > (const api_param& rhs) const {
        return this->key_.compare(rhs.key_) > 0?true:false;
    }
}API_PARAM;

class CDownloadBinlog {
public:
    CDownloadBinlog();
    CDownloadBinlog(const CDownloadBinlog& orig);
    virtual ~CDownloadBinlog();
    void getDownloadList();
private:
    vector<API_PARAM> m_params;
    void buildParams();
    string buildStrToSign();
    string buildStrToRequest();
    string getFormatTime();
    string getRandom();
    int hmacEncode(const char * algo,const char * key, unsigned int key_length,const char * input, unsigned int input_length,unsigned char * &output, unsigned int &output_length);
};

#endif /* CDOWNLOADBINLOG_H */

