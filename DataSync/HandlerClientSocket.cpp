/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HandlerClientSocket.cpp
 * Author: tx
 * 
 * Created on 2016年3月9日, 下午3:19
 */

#include "HandlerClientSocket.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <bits/signum.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <netinet/tcp.h>
#include <AliMessageQueue.h>
#include "SocketServer.h"
#include "LogError.h"
#include "LogData.h"
#include <comm.h>
#include <MyException.h>
#include <lock.h>
#include <Commfun.h>
#include "CProcessMqData.h"

HandlerClientSocket::HandlerClientSocket() {
}

HandlerClientSocket::HandlerClientSocket(int socket, string ip, SocketServer* pSocketServer) {
    m_ip = ip;
    m_p_socket_server = pSocketServer;
    m_client_socket = socket;
    m_client_thread_id = 0;
    m_client_id = "";
    m_last_heartbeat_timestamp = time(0);
}

HandlerClientSocket::HandlerClientSocket(const HandlerClientSocket& orig) {
}

HandlerClientSocket::~HandlerClientSocket() {
    
}

bool HandlerClientSocket::killClientThread() {
    destorySocket();
     if(pthread_cancel(m_client_thread_id) == 0)
         return true;
     return false;
}

void HandlerClientSocket::destorySocket() {
    if(m_client_socket != 0) {
        shutdown(m_client_socket, SHUT_RDWR);
        close(m_client_socket);
        m_client_socket = 0;
    }
}

void* threadHandler(void *arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    HandlerClientSocket *pInstance = (HandlerClientSocket*)arg;
    int rv = -1;
    if(pInstance->initClientSocket() == false) {
        pInstance->destorySocket();
        pthread_exit((void*)&rv);
    }
    if(pInstance->initConnect() == false) {
        pInstance->destorySocket();
        pthread_exit((void*)&rv);
    }
    
    while(true){
        string strRecvData;
        SOCKET_MESSAGE_HEADER msgHeader;
        pthread_testcancel();
        if(pInstance->recvMessage(msgHeader, strRecvData) == false) {
           continue;
        }
        pthread_testcancel();
        pInstance->handlerMessage(msgHeader, strRecvData);
    }
    pInstance->destorySocket();
    pthread_exit((void*)&rv);
}

bool HandlerClientSocket::recvMessage(SOCKET_MESSAGE_HEADER &msgHeader, string &strRecvData) {
        char *pBuf = NULL;
        char buf[MESSAGE_BUFFER_OVER_LEN + 1];
        memset(buf, 0x00, MESSAGE_BUFFER_OVER_LEN + 1);
        int recvLen = 0;
        if(recvLen = mRecv(m_client_socket, (char*)&msgHeader, sizeof(SOCKET_MESSAGE_HEADER)) == sizeof(SOCKET_MESSAGE_HEADER)) {
            int bodyLen = msgHeader.data_len_ - sizeof(SOCKET_MESSAGE_HEADER);
            pBuf = (char*)malloc((bodyLen + 1) * sizeof(char));
            if(pBuf != NULL){
                memset(pBuf, 0x00, bodyLen + 1);
                recvLen = mRecv(m_client_socket, pBuf, bodyLen);
                if(recvLen != bodyLen) {
                    cout<< "body data error"<< endl;
                    return false;
                }
                recvLen = mRecv(m_client_socket, buf, MESSAGE_BUFFER_OVER_LEN);
                if(recvLen != MESSAGE_BUFFER_OVER_LEN) {
                    cout<< "MESSAGE_BUFFER_OVER recv failed"<< endl;
                    return false;
                }
                if(memcmp(buf, MESSAGE_BUFFER_OVER, MESSAGE_BUFFER_OVER_LEN) != 0) {
                    cout<< "MESSAGE_BUFFER_OVER is not cmp:"<< buf<< endl;
                    return false;
                }
            }
            if(pBuf != NULL) {
                strRecvData = string(pBuf);
                free(pBuf);
                return true;
            }
        }
        return false;
}

bool HandlerClientSocket::initClientSocket() {
    int rv = 0;
    struct timeval timeout; 
    timeout.tv_sec = CLIENT_RECV_TIMEOUT;
    timeout.tv_usec = 0;
    rv = setsockopt(m_client_socket, SOL_SOCKET, SO_RCVTIMEO, (void*)&timeout, sizeof(struct timeval));
    if(rv != 0) {
        return false;
    }
    rv = setsockopt(m_client_socket, SOL_SOCKET, SO_SNDTIMEO, (void*)&timeout, sizeof(struct timeval));
    if(rv != 0) {
        return false;
    }
    return true;
}

bool HandlerClientSocket::initConnect() {
    sendMessage(MSG_TYPE_HEART_BEAT, "begin");
    string strRecvData;
    SOCKET_MESSAGE_HEADER msgHeader;
    if(recvMessage(msgHeader, strRecvData) == true) {
        if((msgHeader.code_ == MSG_TYPE_STORECODE) && (handlerMessage(msgHeader, strRecvData) == true)) {
            sendMessage(MSG_TYPE_CONNECT_SUCCESS, "i am server");
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
    return true;
}
bool HandlerClientSocket::run() {
    int rv = 0;
    if(m_client_thread_id == 0) {
        rv = pthread_create(&m_client_thread_id, NULL, threadHandler, (void*)this);
    }
    if(rv == 0) {
        return true;
    }
    else
        return false;
}

int HandlerClientSocket::getClientSocket() {
    return m_client_socket;
}

string HandlerClientSocket::getIpAddress() const{
    return m_ip;
}

void HandlerClientSocket::setIpAddress(string ip) {
    m_ip = ip;
}

string HandlerClientSocket::getClientId() {
    return m_client_id;
}

bool HandlerClientSocket::cmpClientId(const char* clientid) {
    return strcmp(clientid, m_client_id.c_str()) == 0?true:false;
}

SocketServer* HandlerClientSocket::getSocketServer() {
    return m_p_socket_server;
}

bool HandlerClientSocket::sendMessage(SOCKET_MESSAGE_HEADER& header, string strData) {
    int iSendLen = header.data_len_;
    char *pSendBuf = (char*)malloc(iSendLen + 1);
    memset(pSendBuf, 0x00, iSendLen);
    memcpy(pSendBuf, &header, sizeof(SOCKET_MESSAGE_HEADER));
    memcpy(pSendBuf + sizeof(SOCKET_MESSAGE_HEADER), strData.c_str(), strData.length());
    if(mSend(m_client_socket, pSendBuf, iSendLen) < iSendLen) {
        free(pSendBuf);
        std::cout<< header.code_ << ":"<< header.message_id_<< std::endl;
        CLogError::getInstance()->WriteErrorLog(errno, "sendMessage send too small");
        return false;
    }
    free(pSendBuf);
    if(mSend(m_client_socket, MESSAGE_BUFFER_OVER, strlen(MESSAGE_BUFFER_OVER)) != strlen(MESSAGE_BUFFER_OVER)) {
        CLogError::getInstance()->WriteErrorLog(errno, "sendMessage send too small over");
        return false;
    }
    return true;
}
/**/
bool HandlerClientSocket::sendMessage(int type, string strData) {
    int iSendLen = strData.length() + sizeof(SOCKET_MESSAGE_HEADER);
    SOCKET_MESSAGE_HEADER headerData;
    headerData.code_ = type;
    headerData.data_len_ = iSendLen;
    return sendMessage(headerData, strData);
}

bool HandlerClientSocket::handlerMessage(const SOCKET_MESSAGE_HEADER &msgHeader, const string strMessage) {
    int opCode = msgHeader.code_;
    switch(opCode) {
        case MSG_TYPE_STORECODE: {
            if(strMessage.compare("") == 0 || strMessage.length() == 0) {
                return false;
            }
            if(m_p_socket_server->checkClientsIdExist(strMessage) == true) {
                return false;
            }
            m_last_heartbeat_timestamp = time(0);
            m_client_id = strMessage;
            m_p_socket_server->removeDisconnectStore(strMessage);
           cout<< "STORE_ID:<<<"<< strMessage.c_str()<< endl;
            break;
        }
        case MSG_TYPE_HEART_BEAT: {
            m_last_heartbeat_timestamp = time(0);
        //    cout<< "heart beat:"<< m_client_id.c_str()<< endl;
            break;
        }
         case MSG_TYPE_BINLOG:{
            if(strMessage.compare("") == 0 || strMessage.length() == 0) {
                return false;
            }
            CLogData::getInstance()->WriteLogFormat("ServeRecvRawData",getClientId().c_str(), strMessage.c_str(), NULL);
            g_instance_CProcessMqData.ProductRawData(getClientId().c_str(),strMessage.c_str());
           break;
        }
        default: {
            
            break;
        }
    }
    return true;
}