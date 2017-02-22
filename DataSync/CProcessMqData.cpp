/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CProcessMqData.cpp
 * Author: tx
 * 
 * Created on 2016年4月27日, 下午8:32
 */

#include "CProcessMqData.h"
#include "AliMessageQueue.h"
#include <SocketServer.h>
#include <MyException.h>
#include <LogError.h>
#include <LogData.h>

CProcessMqData g_instance_CProcessMqData;
CProcessMqData::CProcessMqData() {
    m_thread_id = 0;
    m_thread_id_raw = 0;
    m_thread_id_exception = 0 ;
}

CProcessMqData::CProcessMqData(const CProcessMqData& orig) {
}

CProcessMqData::~CProcessMqData() {
}
void* CProcessMqData::threadProcess(void* arg) {
    int rv = 0;
    CProcessMqData *pInstance = (CProcessMqData*)arg;
    while(true){
        MQ_DATA *pData = pInstance->ConsumerData();
        if(pData != NULL) {
            SocketServer::getInstance()->dispatchMessage(pData->header_, pData->dispatch_type_, pData->stores_, pData->body_);
        //    cout<< pData->body_.c_str()<< endl;
            delete pData;
        }
        
    }
    
    pthread_exit(&rv);
}
void * CProcessMqData::threadTest(void* args) {
    CProcessMqData *pInstance = (CProcessMqData*)args;
    sleep(3);
    while(true) {
        /*
        for(int i = 0; i < 10; i ++) {
        string dispatch_type_ = "3";
        string stores_ = "";
         string body_ = "{\"tables\":[{\"name\":\"customers\",\"column\":[\"AREA_ID\",\"X_COOR\",\"CONTACT\",\"C_NAME\",\"C_CODE\",\"PHONE\",\"ADDRESS\",\"Y_COOR\",\"WH_CODE\"],\"values\":[\"82725\",\"126.614574\",\"好刚\",\"好刚仓买\",\"230185\",\"84806101\",\"黑龙江省哈尔滨市道里区建国街道建国南二道街副10号\",\"45.757095\",\"190\"],\"mapkey\":{\"C_CODE\":230185}}]}";
        SOCKET_MESSAGE_HEADER header_;
        header_.code_ = MSG_TYPE_UPDATE_DATABASE;
        header_.data_len_ = sizeof(SOCKET_MESSAGE_HEADER) + body_.length();
        memcpy (header_.message_id_ , "0A91892A00001F9000000AAB49E9BF99", 32); 
         pInstance->ProductData(header_, dispatch_type_, stores_, body_);
        }*/
        
         
         string str = "30000{\"tables\":[{\"name\":\"customers\",\"column\":[\"AREA_ID\",\"X_COOR\",\"CONTACT\",\"C_NAME\",\"C_CODE\",\"PHONE\",\"ADDRESS\",\"Y_COOR\",\"WH_CODE\"],\"values\":[\"82725\",\"126.614574\",\"好刚\",\"好刚仓买\",\"230185\",\"84806101\",\"黑龙江省哈尔滨市道里区建国街道建国南二道街副10号\",\"45.757095\",\"190\"],\"mapkey\":{\"C_CODE\":230185}}]}";
         for(int i = 0; i < 10; i++) {
             AliMessageQueue::getInstance()->pushMessage(erp_producer, "tx", "tx", str);
         }
        sleep(2);
    }
    
}

void* CProcessMqData::threadProcessRaw(void* arg) {
    int rv = 0;
    CProcessMqData *pInstance = (CProcessMqData*)arg;
    while(true){
        MQ_DATA *pData = pInstance->ConsumerRawData();
        if(pData!=NULL)
        { 
            AliMessageQueue::getInstance()->pushMessage(wms_producer, pData->stores_, "", pData->body_);
        }
        delete pData;
    }
    
    pthread_exit(&rv);
}

bool CProcessMqData::StartThreadRaw() {
    int rv = 0;
    if(m_thread_id_raw == 0) {
        rv = pthread_create(&m_thread_id_raw, NULL, threadProcessRaw, (void*)this);
    }
    if(rv == 0) {
        return true;
    }
    else {
        THROW_EXCEPTION(ExceptionBase,  "CProcessMqData thread thread raw failed");
        return false;
    }
}

void CProcessMqData::StartThreadProduct() {
    pthread_create(&m_thread_id, NULL, threadTest, (void*)this);
}
bool CProcessMqData::StartThread() {
    int rv = 0;
    if(m_thread_id == 0) {
        rv = pthread_create(&m_thread_id, NULL, threadProcess, (void*)this);
    }
    if(rv == 0) {
        return true;
    }
    else {
        THROW_EXCEPTION(ExceptionBase,  "CProcessMqData thread thread failed");
        return false;
    }
}

bool CProcessMqData::StartThreadException() {
    int rv = 0;
    if(m_thread_id_exception == 0) {
        rv = pthread_create(&m_thread_id_exception, NULL, threadProcessException, (void*)this);
    }
    if(rv == 0) {
        return true;
    }
    else {
        THROW_EXCEPTION(ExceptionBase,  "CProcessMqData thread thread failed");
        return false;
    }
}

void* CProcessMqData::threadProcessException(void* arg) {
    int rv = 0;
    CProcessMqData *pInstance = (CProcessMqData*)arg;
    while(true){
        MQ_DATA *pData = pInstance->ConsumerExceptionData();
        if(pData != NULL) {
            if(pData->stores_.empty())
            {
                continue;
            }
            else
            {
                SocketServer::getInstance()->dispatchOneMessage(pData->header_, pData->dispatch_type_, pData->stores_, pData->body_);
                CLogData::getInstance()->WriteLogFormat("ConsumerExceptionData",  pData->stores_.c_str(), pData->body_.c_str(), NULL);
                delete pData;
            }
            
        }
        
    }
    
    pthread_exit(&rv);
}

void CProcessMqData::ProductData(SOCKET_MESSAGE_HEADER header, const string disType, const string stores, const string body) {
    ENTRY_MUTEX(CProcessMqData);
    MQ_DATA *pData = new MQ_DATA;
    pData->header_ = header;
    pData->dispatch_type_ = disType;
    pData->stores_ = stores;
    pData->body_ = body;
    if(m_data.size() >= MAX_QUEUE_DATA) {
        CONDITION_SLEEP(CProcessMqData, full);
    }
    else {
        m_data.push_back(pData);
    }
    cout<< "mq_queue_data_size======"<< m_data.size()<< endl<< endl;
    CONDITION_WAKEUP(CProcessMqData, empty);
    LEAVE_MUTEX(CProcessMqData);
}

MQ_DATA* CProcessMqData::ConsumerData() {
    ENTRY_MUTEX(CProcessMqData);
    MQ_DATA *retData = NULL;
    if(m_data.size()  == 0) {
        deque<MQ_DATA*>().swap(m_data);
        CONDITION_SLEEP(CProcessMqData, empty);
    }
    if (m_data.size() > 0){
        retData = m_data.front();
        m_data.pop_front();
    }
    CONDITION_WAKEUP(CProcessMqData, full);
    LEAVE_MUTEX(CProcessMqData);
    return retData;
}

void CProcessMqData::ProductExceptionData(SOCKET_MESSAGE_HEADER header, const string disType, const string stores, const string body) {
    MQ_DATA *pData = new MQ_DATA;
    pData->header_ = header;
    pData->dispatch_type_ = disType;
    pData->stores_ = stores;
    pData->body_ = body;
    ENTRY_MUTEX(CProcessMqData);
    if(m_exception_data.size() >= MAX_QUEUE_DATA) {
        CONDITION_SLEEP(CProcessMqData, full);
    }
    else {
        m_exception_data.push_back(pData);
    }
    CONDITION_WAKEUP(CProcessMqData, empty);
    LEAVE_MUTEX(CProcessMqData);
}

MQ_DATA* CProcessMqData::ConsumerExceptionData() {
    ENTRY_MUTEX(CProcessMqData);
    MQ_DATA *retData = NULL;
    if(m_exception_data.size()  == 0) {
        CONDITION_SLEEP(CProcessMqData, empty);
    }
    if (m_exception_data.size() > 0){
        retData = m_exception_data.front();
        m_exception_data.pop_front();
    }
    CONDITION_WAKEUP(CProcessMqData, full);
    LEAVE_MUTEX(CProcessMqData);
    return retData;
}


void CProcessMqData::ProductRawData(const string stores,const string body) {
    MQ_DATA *pData = new MQ_DATA;
    pData->body_ = body;
    pData->stores_ = stores;
    ENTRY_MUTEX(CProcessMqRawData);
    if(m_raw_data.size() >= MAX_QUEUE_DATA) {
        CONDITION_SLEEP(CProcessMqRawData, full);
    }
    else {
        m_raw_data.push_back(pData);
    }
    CONDITION_WAKEUP(CProcessMqRawData, empty);
    LEAVE_MUTEX(CProcessMqRawData);
}

MQ_DATA* CProcessMqData::ConsumerRawData() {
    ENTRY_MUTEX(CProcessMqRawData);
    MQ_DATA *retData;
    if(m_raw_data.size()  == 0) {
        CONDITION_SLEEP(CProcessMqRawData, empty);
    }
    if (m_raw_data.size() > 0){
        retData = m_raw_data.front();
        m_raw_data.pop_front();
    }
    CONDITION_WAKEUP(CProcessMqRawData, full);
    LEAVE_MUTEX(CProcessMqRawData);
    return retData;
}