/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   AliMessageQueue.cpp
 * Author: tx
 * 
 * Created on 2016年3月14日, 下午7:54
 */

#include "AliMessageQueue.h"
#include "LogData.h"

#include <lock.h>
#include <LogError.h>
#include <string.h>
#include <comm.h>
#include <MyMessageListener.h>
#include <Config.h>
#include <MyException.h>


AliMessageQueue* AliMessageQueue::m_instance = NULL;
AliMessageQueue::AliMessageQueue() {
    initConfig();
}

AliMessageQueue::AliMessageQueue(const AliMessageQueue& orig) {
}

AliMessageQueue::~AliMessageQueue() {
    map<QUEUE_INSTANCE, P_QUEUE_DATA>::iterator it;
    for(it = m_queueInfos.begin(); it != m_queueInfos.end(); it++) {
        if((*it).second->producer_ != NULL)
            (*it).second->producer_->shutdown();
        if((*it).second->consumer_ != NULL)
            (*it).second->consumer_->shutdown();
        if((*it).second->msglistener_ != NULL)
            delete ((*it).second->msglistener_);
        delete ((*it).second);
    }
    m_queueInfos.erase(m_queueInfos.begin(), m_queueInfos.end());
}
bool AliMessageQueue::initConfig(const char* szPath) {
    Config config(szPath);
    char szTmp[1024];
    int rv = 0;
    memset(szTmp, 0x00, 1024);
    MQ_CONFIG tmpConfig;
    rv = config.GetConfigStringValue("MQ_FINAL_TOPIC", "name", szTmp);
    if(rv != SUCCESS) {
        THROW_EXCEPTION(ExceptionMessageQueue, "config MQ_FINAL_TOPIC name");
        return false;
    }
    tmpConfig.name_ = szTmp;
    
    memset(szTmp, 0x00, 1024);
    rv = config.GetConfigStringValue("MQ_FINAL_TOPIC", "producer", szTmp);
    if(rv != SUCCESS) {
        THROW_EXCEPTION(ExceptionMessageQueue, "config MQ_FINAL_TOPIC producer");
        return false;
    }
    tmpConfig.producer_ = szTmp;
    
    memset(szTmp, 0x00, 1024);
    rv = config.GetConfigStringValue("MQ_FINAL_TOPIC", "consumer", szTmp);
    if(rv != SUCCESS) {
        THROW_EXCEPTION(ExceptionMessageQueue, "config MQ_FINAL_TOPIC consumer");
        return false;
    }
    tmpConfig.consumer_ = szTmp;
    m_mq_config.insert(pair<TOPIC_ENUM, MQ_CONFIG>(erp_final_topic, tmpConfig));
    
    //whc 上行
    memset(szTmp, 0x00, 1024);
    rv = config.GetConfigStringValue("MQ_RAW_TOPIC", "name", szTmp);
    if(rv != SUCCESS) {
        THROW_EXCEPTION(ExceptionMessageQueue, "config MQ_RAW_TOPIC name");
	return false;
    }
    m_wms_topic_name = szTmp;
    tmpConfig.name_ = szTmp;

    memset(szTmp, 0x00, 1024);
    rv = config.GetConfigStringValue("MQ_RAW_TOPIC", "producer", szTmp);
    if(rv != SUCCESS) {
	THROW_EXCEPTION(ExceptionMessageQueue, "config MQ_RAW_TOPIC producer");
	return false;
    }
    tmpConfig.producer_ = szTmp;

    memset(szTmp, 0x00, 1024);
    rv = config.GetConfigStringValue("MQ_RAW_TOPIC", "consumer", szTmp);
    if(rv != SUCCESS) {
	THROW_EXCEPTION(ExceptionMessageQueue, "config MQ_RAW_TOPIC consumer");
	return false;
    }
    tmpConfig.consumer_ = szTmp;
    m_mq_config.insert(pair<TOPIC_ENUM, MQ_CONFIG>(queue_raw_topic, tmpConfig));	
    //end whc    
    
    memset(szTmp, 0x00, 1024);
    rv = config.GetConfigStringValue("MQ_EXCEPTION", "name", szTmp);
    if(rv != SUCCESS) {
        THROW_EXCEPTION(ExceptionMessageQueue, "config MQ_EXCEPTION name");
        return false;
    }
    tmpConfig.name_ = szTmp;
    
    memset(szTmp, 0x00, 1024);
    rv = config.GetConfigStringValue("MQ_EXCEPTION", "producer", szTmp);
    if(rv != SUCCESS) {
        THROW_EXCEPTION(ExceptionMessageQueue, "config MQ_EXCEPTION producer");
        return false;
    }
    tmpConfig.producer_ = szTmp;
    
    memset(szTmp, 0x00, 1024);
    rv = config.GetConfigStringValue("MQ_EXCEPTION", "consumer", szTmp);
    if(rv != SUCCESS) {
        THROW_EXCEPTION(ExceptionMessageQueue, "config MQ_EXCEPTION consumer");
        return false;
    }
    tmpConfig.consumer_ = szTmp;
    m_mq_config.insert(pair<TOPIC_ENUM, MQ_CONFIG>(exception_topic, tmpConfig));
    return true;
}
AliMessageQueue* AliMessageQueue::getInstance() {
    MUTEX_LOCK(AliMessageQueue);
    if(AliMessageQueue::m_instance == NULL) {
        AliMessageQueue::m_instance = new AliMessageQueue();
    }
    return AliMessageQueue::m_instance;
}

bool AliMessageQueue::pushMessage(QUEUE_INSTANCE enumState, CONSTSTRING strTag, CONSTSTRING strKey, CONSTSTRING strMessage) {
    P_QUEUE_DATA pQueueData = getProOrCon(enumState);
    if(pQueueData == NULL)
        return false;
    Message msg;
    msg.setTag(strTag);
    msg.setKey(strKey);
    msg.setTopic(pQueueData->factoryInfo_.getPublishTopics());
    msg.setBody(strMessage);
    
    try {
        SendResultONS sendResult = pQueueData->producer_->send(msg);
    }
    catch(ONSClientException & e) {
        CLogError::getInstance()->WriteErrorLog(e.GetError(), e.GetMsg());
        return false;
    }
    return true;
}

bool AliMessageQueue::pushRawMessage(QUEUE_INSTANCE enumState, CONSTSTRING strTag, CONSTSTRING strKey, CONSTSTRING strMessage) {
    P_QUEUE_DATA pQueueData = getProOrCon(enumState);
    if(pQueueData == NULL)
        return false;
    Message msg;
    msg.setTag(strTag);
    msg.setKey(strKey);
    msg.setTopic(pQueueData->factoryInfo_.getPublishTopics());
    msg.setBody(strMessage);
    
    try {
        SendResultONS sendResult = pQueueData->producer_->send(msg);
    }
    catch(ONSClientException & e) {
        CLogError::getInstance()->WriteErrorLog(e.GetError(), e.GetMsg());
        return false;
    }
    return true;
}


Producer *AliMessageQueue::createProducer(const char* szTopics, const char* szProducerId, ONSFactoryProperty& factoryInfo) {
    factoryInfo.setFactoryProperty(ONSFactoryProperty::AccessKey, ONS_ACCESS_KEY);
    factoryInfo.setFactoryProperty(ONSFactoryProperty::SecretKey, ONS_SECRET_KEY);
    factoryInfo.setFactoryProperty(ONSFactoryProperty::PublishTopics, szTopics);
    factoryInfo.setFactoryProperty(ONSFactoryProperty::ProducerId, szProducerId);
    Producer* pProducer = ONSFactory::getInstance()->createProducer(factoryInfo);
    pProducer->start();
    return pProducer;
}

PushConsumer *AliMessageQueue::createConsumer(const char* szTopics, const char* szConsumer, ONSFactoryProperty& factoryInfo) {
    factoryInfo.setFactoryProperty(ONSFactoryProperty::AccessKey, ONS_ACCESS_KEY);
    factoryInfo.setFactoryProperty(ONSFactoryProperty::SecretKey, ONS_SECRET_KEY);
    factoryInfo.setFactoryProperty(ONSFactoryProperty::PublishTopics, szTopics);
    factoryInfo.setFactoryProperty(ONSFactoryProperty::ConsumerId, szConsumer);
    PushConsumer *pConsumer = ONSFactory::getInstance()->createPushConsumer(factoryInfo);
    return pConsumer;
}

P_QUEUE_DATA AliMessageQueue::getProOrCon(QUEUE_INSTANCE enumState) {
    P_QUEUE_DATA pQueueData =  NULL;
    if(producerIsExist(enumState) == true) {
        pQueueData = m_queueInfos[enumState];
    }
    else {
        pQueueData = new QUEUE_DATA;
        ONSFactoryProperty& factoryInfo = pQueueData->factoryInfo_;
        pQueueData->producer_ = NULL;
        pQueueData->consumer_ = NULL;
        pQueueData->msglistener_ = NULL;
        switch(enumState) {
            case wms_producer:  {
                string strTopic = m_mq_config[queue_raw_topic].name_;
                string strProducer = m_mq_config[queue_raw_topic].producer_;
                pQueueData->producer_ = createProducer(strTopic.c_str(), strProducer.c_str(), factoryInfo);
                break;
            }
            case erp_producer: {
                string strTopic = m_mq_config[erp_final_topic].name_;
                string strProducer = m_mq_config[erp_final_topic].producer_;
                pQueueData->producer_ = createProducer(strTopic.c_str(), strProducer.c_str(), factoryInfo);
                break;
            }
            case exception_thread1_producer: {
                string strTopic = m_mq_config[exception_topic].name_;
                string strProducer = m_mq_config[exception_topic].producer_;
                pQueueData->producer_ = createProducer(strTopic.c_str(), strProducer.c_str(), factoryInfo);
                break;
            }
            case wms_consumer: {
                pQueueData->consumer_ = createConsumer(ONS_WMS_TOPIC, ONS_WMS_CONSUMER, factoryInfo);
                break;
            }
            case erp_consumer: {
                pQueueData->consumer_ = createConsumer(ONS_ERP_TOPIC, ONS_ERP_CONSUMER, factoryInfo);
                break;
            }
            case erp_final_consumer: {
                string strTopic = m_mq_config[erp_final_topic].name_;
                string strConsumer = m_mq_config[erp_final_topic].consumer_;
                pQueueData->consumer_ = createConsumer(strTopic.c_str(), strConsumer.c_str(), factoryInfo);
                pQueueData->msglistener_ = new MessageListenerErpFinal();
                break;
            }
            case exception_thread1_consumer: {
                string strTopic = m_mq_config[exception_topic].name_;
                string strConsumer = m_mq_config[exception_topic].consumer_;
                pQueueData->consumer_ = createConsumer(strTopic.c_str(), strConsumer.c_str(), factoryInfo);
                pQueueData->msglistener_ = new MessageListenerException();
                break;
            }
            default: {
                break;
            }
        }
        if(pQueueData->producer_ != NULL || pQueueData->consumer_ != NULL) {
            m_queueInfos.insert(pair<QUEUE_INSTANCE, P_QUEUE_DATA>(enumState, pQueueData));
        }
    }
    return pQueueData;
}

bool AliMessageQueue::producerIsExist(QUEUE_INSTANCE enumState) {
    return (m_queueInfos.find(enumState) != m_queueInfos.end());
}

bool AliMessageQueue::runServer(QUEUE_INSTANCE enumState) {
    try {
        P_QUEUE_DATA pQueueData = getProOrCon(enumState);
        if(pQueueData->consumer_ == NULL) {
            return false;
        }
        PushConsumer* pConsumer = pQueueData->consumer_;
        MessageListener* pMsgListener = pQueueData->msglistener_;
        pConsumer->subscribe(pQueueData->factoryInfo_.getPublishTopics(), "*", pMsgListener);
        pConsumer->start();
        cout<< "aliMQ run server success"<< endl;
    }
    catch(ONSClientException & e) {
        CLogError::getInstance()->WriteErrorLog(e.GetError(), e.GetMsg());
        return false;
    }
    return true;
}