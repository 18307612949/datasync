/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Config.h
 * Author: tx
 *
 * Created on 2016年1月25日, 下午1:34
 */

#ifndef CONFIG_H
#define CONFIG_H
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;

#define CONFIG_FILE_PATH "/var/gdconfig/gd_bank"
#define  SUCCESS           0x00 /*成功*/  
#define  FAILURE           0x01 /*失败*/  
  
#define  FILENAME_NOTEXIST      0x02 /*配置文件名不存在*/  
#define  SECTIONNAME_NOTEXIST    0x03 /*节名不存在*/  
#define  KEYNAME_NOTEXIST      0x04 /*键名不存在*/  
#define  STRING_LENNOTEQUAL     0x05 /*两个字符串长度不同*/  
#define  STRING_NOTEQUAL       0x06 /*两个字符串内容不相同*/  
#define  STRING_EQUAL        0x00 /*两个字符串内容相同*/

class Config {
public:
    Config();
    Config(const Config& orig);
    Config(const char* szConfigPath);
    virtual ~Config();
    
    int GetKeyValue(FILE *fpConfig,const char *pInKeyName,char *pOutKeyValue);  
    int GetConfigIntValue(char *pInSectionName,const char *pInKeyName,int *pOutKeyValue);  
    int GetConfigStringValue(char *pInSectionName,const char *pInKeyName,char *pOutKeyValue);  
private:
    int CompareString(const char *pInStr1, const char *pInStr2);  
    string m_configPath;
};

#endif /* CONFIG_H */

