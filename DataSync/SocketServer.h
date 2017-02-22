/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SocketServer.h
 * Author: tx
 *
 * Created on 2016年3月8日, 下午3:57
 */

#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H
class HandlerClientSocket;
#include <pthread.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <defines.h>
#include <socket_message.h>
using namespace std;
typedef vector<HandlerClientSocket*> HANDLER_CLIENT_HASH;
class SocketServer {
public:
    bool run();
    void destoryClientSockets();
    HANDLER_CLIENT_HASH& getHandlerClients();
    bool addClient(HandlerClientSocket* client);
    void removeClient(HandlerClientSocket* client);
    static SocketServer* getInstance();
    static void* heartBeatThread(void* args);
    bool runHeartBeatThread();
    virtual ~SocketServer();
    void sendHeartbeat();
    void dispatchMessage(SOCKET_MESSAGE_HEADER& header, string type, string storeCode, string strData);
    void dispatchOneMessage(SOCKET_MESSAGE_HEADER& header, string type, string storeCode, string strData);
    void dealDisconnectSocket(int socketId);
    void addDisconnectStore(int socketId);
    void removeDisconnectStore(const string& storeId);
    bool checkDisconnectStore(const string& storeId);
    void dealDisconnectStoreData(const string& strData);
    void dealDisconnectStoreData(const string &storeId, const string &strData);
    void dealDisconnectStoreData(const vector<string> &storeId, const string &strData);
    bool checkClientsIpExist(string ip); //检测是否有该ip的client
    bool checkClientsIdExist(string id); //检测是否有该id的client
    static SocketServer *m_instance;
    void showDisConnectStore();
    void waitServerThread();
private:
    SocketServer();
    SocketServer(const SocketServer& orig);
    void dealSendFailed(const string &storeCode, const string &strData);
    inline bool checkSocketState(int socketId);
    pthread_t m_server_thread_id;
    pthread_t m_heartbeat_thread_id;
    int m_server_thread_exit_code;
    HANDLER_CLIENT_HASH m_clients;
    vector<string> m_disconnect_store;
//    ErrorLog m_error_instance;
};
extern void ClientStateCheck(int code);
#endif /* SOCKETSERVER_H */

