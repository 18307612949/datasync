/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CProcessMqData.h
 * Author: tx
 *
 * Created on 2016年4月27日, 下午8:32
 */

#ifndef CPROCESSMQDATA_H
#define CPROCESSMQDATA_H
#include <iostream>
#include <deque>
#include <socket_message.h>
#include <pthread.h>
#include <lock.h>
using namespace std;
#define MAX_QUEUE_DATA 1000
typedef struct mq_data{
    SOCKET_MESSAGE_HEADER header_;
    string dispatch_type_;
    string stores_;
    string body_;
}MQ_DATA;
class CProcessMqData {
public:
    CProcessMqData();
    CProcessMqData(const CProcessMqData& orig);
    virtual ~CProcessMqData();
    static void* threadProcess(void *arg);
    static void* threadProcessException(void *arg);
    static void* threadProcessRaw(void *arg);
    static void* threadTest(void *args);
    int getQueueDataSize(){return m_data.size();}
    bool StartThread();
    bool StartThreadRaw();
    bool StartThreadException();
    void StartThreadProduct();
    void ProductData(SOCKET_MESSAGE_HEADER header, const string disType, const string stores, const string body);
    void ProductExceptionData(SOCKET_MESSAGE_HEADER header, const string disType, const string stores, const string body);
    void ProductRawData(const string stores,const string body);
    MQ_DATA* ConsumerData();
    MQ_DATA* ConsumerExceptionData();
    MQ_DATA* ConsumerRawData();
private:
    deque<MQ_DATA*> m_data;
    deque<MQ_DATA*> m_raw_data;
    deque<MQ_DATA*> m_exception_data;
    pthread_t m_thread_id;
    pthread_t m_thread_id_raw;
    pthread_t m_thread_id_exception;
};
extern CProcessMqData g_instance_CProcessMqData;
#endif /* CPROCESSMQDATA_H */

