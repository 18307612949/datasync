/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HandlerClientSocket.h
 * Author: tx
 *
 * Created on 2016年3月9日, 下午3:19
 */

#ifndef HANDLERCLIENTSOCKET_H
#define HANDLERCLIENTSOCKET_H
class SocketServer;
#include <pthread.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <defines.h>
#include "socket_message.h"
class HandlerClientSocket {
public:
    
    HandlerClientSocket(int socket, string ip, SocketServer* pSocketServer);
    HandlerClientSocket(const HandlerClientSocket& orig);
    virtual ~HandlerClientSocket();
    bool run();
    int getClientSocket(); //获取该实例的client的socket
    string getClientId(); // 获取client的仓库id
    string getIpAddress() const;
    void setIpAddress(string ip);
    bool cmpClientId(const char* clientid);
    SocketServer* getSocketServer(); //获取socket服务指针
    bool handlerMessage(const SOCKET_MESSAGE_HEADER &msgHeader, const string strMessage);
    bool recvMessage(SOCKET_MESSAGE_HEADER &header, string &strData);
    bool sendMessage(int type, string strData);
    bool sendMessage(SOCKET_MESSAGE_HEADER &header, string strData);
    bool initConnect();
    bool initClientSocket();
    bool checkSocketEnabled();
    bool killClientThread();
    void destorySocket();
    long long m_last_heartbeat_timestamp;
    bool m_b_thread_run;
private:
    HandlerClientSocket();
    

    pthread_t m_client_thread_id;
    int m_client_socket;
    string m_client_id;                  //客户端ID 既仓库ID
    int m_thread_exit_id;
    
    int m_lost_data_count;
    SocketServer *m_p_socket_server;
    string m_ip;
};

#endif /* HANDLERCLIENTSOCKET_H */

