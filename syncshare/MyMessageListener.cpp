/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MyMessageListener.cpp
 * Author: tx
 * 
 * Created on 2016年3月21日, 下午4:02
 */
#include "MyMessageListener.h"
#include "LogData.h"
#include <iostream>
#include <SocketServer.h>
#include <socket_message.h>
#include <cstdlib>
#include <CProcessMqData.h>
#include <comm.h>
using namespace std;

MessageListenerErp::MessageListenerErp() {
}

MessageListenerErp::MessageListenerErp(const MessageListenerErp& orig) {
}

MessageListenerErp::~MessageListenerErp() {
}

Action MessageListenerErp::consume(Message &message, ConsumeContext &context) {
    cout<< message.getMsgID()<< ":"<< message.getBody().c_str()<< endl;
    return CommitMessage;
}

MessageListenerErpFinal::MessageListenerErpFinal(){
}
MessageListenerErpFinal::~MessageListenerErpFinal(){
}
Action MessageListenerErpFinal::consume(Message &message, ConsumeContext &context) {
//    cout<< message.getMsgID()<< ":"<< message.getBody().c_str()<< endl;
    
    
    string utf8Str = message.getBody();
//    gbk2utf8(utf8Str, message.getBody().c_str());
    cout<< message.getMsgID()<< ":"<< utf8Str.c_str()<< ":"<< message.getTag().c_str()<< ":"<< message.getKey().c_str()<< endl;
 
    CLogData::getInstance()->WriteLogFormat("consumerData", message.getMsgID().c_str(), message.getBody().c_str(), NULL);
    const string &strMsg = utf8Str;
    const string &strMsgId = message.getMsgID();
    int msgLen = strMsg.length();
    if(msgLen < 5){
        return CommitMessage;
    }
    string strDispatchType = strMsg.substr(0, 1);
    string strStoreLen = strMsg.substr(1, 4);
    int iStoreLen = atoi(strStoreLen.c_str());
    if(msgLen < iStoreLen + 5) {
        return CommitMessage;
    }
    string strStores = strMsg.substr(5, iStoreLen);
    string strBody = strMsg.substr(iStoreLen + 5);
    SOCKET_MESSAGE_HEADER header;
    header.code_ = MSG_TYPE_UPDATE_DATABASE;
    header.data_len_ = sizeof(SOCKET_MESSAGE_HEADER) + strBody.length();
    memcpy(header.message_id_, strMsgId.c_str(), strMsgId.length());
    header.message_id_[strMsgId.length()] = 0;
    
 //  g_instance_CProcessMqData.ProductData(header, strDispatchType, strStores, strBody);
    return CommitMessage;
}


MessageListenerException::MessageListenerException(){}
MessageListenerException::~MessageListenerException(){}
Action MessageListenerException::consume(Message &message, ConsumeContext &context) {
    string utf8Str = message.getBody();
    const string &strMsg = utf8Str;
    int msgLen = strMsg.length();
    if(msgLen < 5){
        return CommitMessage;
    }
    string strDispatchType = "1";//单仓
    string strStores = message.getKey();
    string strBody = message.getBody();
    SOCKET_MESSAGE_HEADER header;
    header.code_ = MSG_TYPE_UPDATE_DATABASE;
    header.data_len_ = sizeof(SOCKET_MESSAGE_HEADER) + strBody.length();
    cout<< message.getKey().c_str()<< endl<< strBody.c_str()<< endl;
    if(strStores.empty())
    {
        return CommitMessage;
    }
    g_instance_CProcessMqData.ProductExceptionData(header, strDispatchType, strStores, strBody);
    return CommitMessage;
}