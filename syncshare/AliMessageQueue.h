/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   AliMessageQueue.h
 * Author: tx
 *
 * Created on 2016年3月14日, 下午7:54
 */

#ifndef ALIMESSAGEQUEUE_H
#define ALIMESSAGEQUEUE_H
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <mq/ONSFactory.h>
#include <mq/ONSClientException.h>
#include <defines.h>
#include <SocketServer.h>
#define MQ_CONFIG_PATH "config/mq_config"
using namespace std;
using namespace ons;
typedef struct queueData{
    Producer* producer_ = NULL;
    PushConsumer* consumer_ = NULL;
    MessageListener* msglistener_ = NULL;
    ONSFactoryProperty factoryInfo_;
    char szTag_[256];
}QUEUE_DATA, *P_QUEUE_DATA;
typedef struct mq_config{
    string name_;
    string producer_;
    string consumer_;
}MQ_CONFIG, *P_MQ_CONFIG;
class AliMessageQueue {
public:
    static AliMessageQueue* getInstance();
    bool pushMessage(QUEUE_INSTANCE enumState, CONSTSTRING strTag, CONSTSTRING strKey, CONSTSTRING strMessage);
    bool pushRawMessage(QUEUE_INSTANCE enumState, CONSTSTRING strTag, CONSTSTRING strKey, CONSTSTRING strMessage);
    bool runServer(QUEUE_INSTANCE enumState);
    virtual ~AliMessageQueue();
    static AliMessageQueue* m_instance;
private:
    AliMessageQueue();
    AliMessageQueue(const AliMessageQueue& orig);
    bool initConfig(const char* szPath = MQ_CONFIG_PATH);
    
    bool produceMessage();
    P_QUEUE_DATA getProOrCon(QUEUE_INSTANCE enumState);
    Producer *createProducer(const char* szTopics, const char* szProducerId, ONSFactoryProperty& factoryInfo);
    PushConsumer* createConsumer(const char* szTopics, const char* szConsumer, ONSFactoryProperty& factoryInfo);
    bool producerIsExist(QUEUE_INSTANCE enumState);
    map<QUEUE_INSTANCE, P_QUEUE_DATA> m_queueInfos;
    map<TOPIC_ENUM, MQ_CONFIG> m_mq_config;
    string m_wms_topic_name; 
};

#endif /* ALIMESSAGEQUEUE_H */

