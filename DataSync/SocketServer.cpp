/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SocketServer.cpp
 * Author: tx
 * 
 * Created on 2016年3月8日, 下午3:57
 */

#include "SocketServer.h"
#include <algorithm>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <netinet/tcp.h>
#include "HandlerClientSocket.h"
#include "LogError.h"
#include "LogData.h"
#include <lock.h>
#include <comm.h>
#include <cstdio>
#include <AliMessageQueue.h>
#include <MyException.h>

SocketServer* SocketServer::m_instance = NULL;
SocketServer::SocketServer() {
    m_server_thread_id = 0;
}

SocketServer::SocketServer(const SocketServer& orig) {
}

SocketServer::~SocketServer() {
    destoryClientSockets();
}
SocketServer* SocketServer::getInstance() {
    if(m_instance == NULL) {
        MUTEX_LOCK(SocketServer);
        if(m_instance == NULL)
            m_instance = new SocketServer();
    }
    return m_instance;
}

void* threadRunServer(void *arg) {
    SocketServer *pInstance = (SocketServer*)arg;
    int serverSocket,  clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t sin_size;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == serverSocket){
        pthread_exit(&errno);
    }
    
    bzero(&serverAddr,sizeof(struct sockaddr_in));
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    serverAddr.sin_port=htons(SERVER_PORT);
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt, sizeof(opt));
    if(-1 == bind(serverSocket, (struct sockaddr*)(&serverAddr), sizeof(struct sockaddr))) {
        close(serverSocket);
        pthread_exit(&errno);
    }

    if(-1 == listen(serverSocket, SERVER_LISTEN_NUM)) {
        close(serverSocket);
        pthread_exit(&errno);
    }

    while(true) {
        sin_size = sizeof(struct sockaddr_in);
        clientSocket = accept(serverSocket, (struct sockaddr*)(&clientAddr), &sin_size); 
        if(-1 == clientSocket) {
            close(serverSocket);
            pthread_exit(&errno);
        }
        string clientIp = inet_ntoa(clientAddr.sin_addr);
        if(pInstance->checkClientsIpExist(clientIp) == true) {
            continue;
        }
        try{
            HandlerClientSocket *pHandler = new HandlerClientSocket(clientSocket, clientIp, pInstance);
            pInstance->addClient(pHandler);
            pHandler->run();
        }
        catch(ExceptionSocket &e){
            cout<< e.what()<< endl;
            CLogError::getInstance()->WriteErrorLog(-1, e.what());
        }
    }
    close(serverSocket);
    int rv = 0;
    pthread_exit(&rv);
}
void SocketServer::waitServerThread() {
    if(m_server_thread_id != 0) {
        pthread_join(m_server_thread_id, (void**)&m_server_thread_exit_code);
    }
}
bool SocketServer::run() {
    int rv = 0;
    if(m_server_thread_id == 0) {
        rv = pthread_create(&m_server_thread_id, NULL, threadRunServer, (void*)this);
    }
    if(rv == 0) {
        return true;
    }
    else {
        THROW_EXCEPTION(ExceptionSocket,  "server thread thread failed");
        return false;
    }
}

HANDLER_CLIENT_HASH& SocketServer::getHandlerClients() {
    return m_clients;
}

bool SocketServer::addClient(HandlerClientSocket* client) {
    m_clients.push_back(client);
    return true;
}

void SocketServer::destoryClientSockets() {
    HANDLER_CLIENT_HASH::iterator it;
    for(it = m_clients.begin(); it != m_clients.end(); it++) {
        delete (*it);
    }
    m_clients.erase(m_clients.begin(), m_clients.end());
}
inline bool SocketServer::checkSocketState(int socketId) {
    struct tcp_info info;
    int len=sizeof(info);
    memset(&info,0,sizeof(info));
    getsockopt(socketId, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len); 
    if(info.tcpi_state!= TCP_ESTABLISHED) 
        return false;
    else
        return true;
}

void SocketServer::dealDisconnectSocket(int socketId) {
    HANDLER_CLIENT_HASH::iterator it;
    for(it = m_clients.begin(); it!= m_clients.end(); it++) {
        if(((HandlerClientSocket*)(*it))->getClientSocket() == socketId) {
            m_disconnect_store.push_back((*it)->getClientId());
            delete (HandlerClientSocket*)(*it);
            m_clients.erase(it);
            break;
        }
    }
}

void SocketServer::addDisconnectStore(int socketId){
    HANDLER_CLIENT_HASH::iterator it;
    for(it = m_clients.begin(); it!= m_clients.end(); it++) {
        if(((HandlerClientSocket*)(*it))->getClientSocket() == socketId) {
            m_disconnect_store.push_back((*it)->getClientId());
            m_clients.erase(it);
            break;
        }
    }
}

void SocketServer::removeDisconnectStore(const string& storeId) {
    vector<string>::iterator it = std::find(m_disconnect_store.begin(), m_disconnect_store.end(), storeId);
    if(it != m_disconnect_store.end()) 
        m_disconnect_store.erase(it);
}

bool SocketServer::checkDisconnectStore(const string& storeId) {
    vector<string>::iterator it = std::find(m_disconnect_store.begin(), m_disconnect_store.end(), storeId);
    return (it == m_disconnect_store.end())? false:true;
}


void SocketServer::dispatchMessage(SOCKET_MESSAGE_HEADER& header, string type, string storeCode, string strData) {
    HANDLER_CLIENT_HASH::iterator it;
    HandlerClientSocket *pDispatch = NULL;
    if(type.compare("1") == 0) {
        for(it = m_clients.begin(); it != m_clients.end(); it++) {
            pDispatch = (*it);
            if(pDispatch->cmpClientId(storeCode.c_str()) == true) {
                 pDispatch->sendMessage(header, strData);
                 break;
            }
        }
        dealDisconnectStoreData(storeCode, strData);
    }
    else if(type.compare("2") == 0) {
        vector<string> vStore = splitStrToVector(storeCode);
        for(it = m_clients.begin(); it != m_clients.end(); it++) {
            pDispatch = (*it);
            if(find(vStore.begin(), vStore.end(), pDispatch->getClientId()) != vStore.end()) {
                pDispatch->sendMessage(header, strData);
            }
        }
        dealDisconnectStoreData(vStore, strData);
    }
    else if(type.compare("3") == 0) {
        for(it = m_clients.begin(); it != m_clients.end(); it++) {
            pDispatch = (*it);
            pDispatch->sendMessage(header, strData);
        }
        dealDisconnectStoreData(strData);
    }
    else {
        
    }
}

void SocketServer::dispatchOneMessage(SOCKET_MESSAGE_HEADER& header, string type, string storeCode, string strData) {
    HANDLER_CLIENT_HASH::iterator it;
    HandlerClientSocket *pDispatch = NULL;
    if(type.compare("1") == 0) {
        for(it = m_clients.begin(); it != m_clients.end(); it++) {
            pDispatch = (*it);
            if(pDispatch->cmpClientId(storeCode.c_str()) == true) {
                 pDispatch->sendMessage(header, strData);
                 break;
            }
        }
        if(it == m_clients.end())//dealDisconnectStoreData(storeCode, strData);
        {
            CLogData::getInstance()->WriteLogFormat("failedExceptionData", storeCode.c_str(), strData.c_str(), NULL);
            AliMessageQueue::getInstance()->pushMessage(exception_thread1_producer, "ERP_SEND_TO_WMS_FAILED", storeCode, strData);
        }
    }
}


void SocketServer::dealDisconnectStoreData(const vector<string>& storeId, const string& strData) {
    vector<string>::iterator it;
    for(it = m_disconnect_store.begin(); it != m_disconnect_store.end(); it++) {
        if(std::find(storeId.begin(), storeId.end(), (*it)) != storeId.end()) {
            dealSendFailed((*it), strData);
        }
    }
}

void SocketServer::dealDisconnectStoreData(const string& storeId, const string& strData) {
    vector<string>::iterator it;
    for(it = m_disconnect_store.begin(); it != m_disconnect_store.end(); it++) {
        if(storeId.compare((*it)) == 0) {
            dealSendFailed((*it), strData);
            break;
        }
    }
}
void SocketServer::dealDisconnectStoreData(const string& strData) {
    vector<string>::iterator it;
    for(it = m_disconnect_store.begin(); it != m_disconnect_store.end(); it++) {
        dealSendFailed((*it), strData);
    }
}

void SocketServer::dealSendFailed(const string &storeCode, const string &strData) {
    cout<< "push mq"<< strData.c_str()<< endl;
    CLogData::getInstance()->WriteLogFormat("failedData", storeCode.c_str(), strData.c_str(), NULL);
    AliMessageQueue::getInstance()->pushMessage(exception_thread1_producer, "ERP_SEND_TO_WMS_FAILED", storeCode, strData);
}

void SocketServer::sendHeartbeat(){
    HANDLER_CLIENT_HASH::iterator it;
    HandlerClientSocket *pWork = NULL;
    MUTEX_LOCK(SocketServerClientManager);
    cout<< "client size:"<< m_clients.size()<< endl;
    for(it = m_clients.begin(); it != m_clients.end(); ) {
        pWork = (*it);
        int socketId = pWork->getClientSocket();
        long long newTime = time(0);
        if(checkSocketState(socketId) == false || (newTime - pWork->m_last_heartbeat_timestamp > 18)) {
            cout<< "remove socket"<< socketId<<"::"<< pWork->getClientId().c_str()<< endl;
            string errStr = "client disconnect storeId[";
            errStr += pWork->getClientId();
            errStr += "]";
            CLogError::getInstance()->WriteErrorLog(-1, errStr);
            removeClient(pWork);
            it = m_clients.erase(it);
            continue;
        }
        else{
            cout<< "clients ==== "<< pWork->getClientId().c_str()<< endl;
            pWork->sendMessage(MSG_TYPE_HEART_BEAT, "hello");
            removeDisconnectStore(pWork->getClientId());
            ++it;
        }
           
    }
}

void SocketServer::removeClient(HandlerClientSocket* client) {
    client->setIpAddress("");
    client->killClientThread();
    string storeId = client->getClientId();
    if(storeId.compare("") != 0) {
        m_disconnect_store.push_back(storeId);
    }
    delete client;
}

bool SocketServer::checkClientsIpExist(string ip) {
    HANDLER_CLIENT_HASH::iterator it;
    HandlerClientSocket *pWork = NULL;
    MUTEX_LOCK(SocketServerClientManager);
    for(it = m_clients.begin(); it != m_clients.end(); it++) {
        pWork = (*it);
        if(pWork->getIpAddress().compare(ip) == 0) {
            return false;
        }
    }
    return false;
}

bool SocketServer::checkClientsIdExist(string id) {
    HANDLER_CLIENT_HASH::iterator it;
    HandlerClientSocket *pWork = NULL;
    MUTEX_LOCK(SocketServerClientManager);
    for(it = m_clients.begin(); it != m_clients.end(); it++) {
        pWork = (*it);
        if(pWork->getClientId().compare(id) == 0) {
            return true;
        }
    }
    return false;
}

void SocketServer::showDisConnectStore(){
    for(int i = 0; i < m_disconnect_store.size(); i++) {
        cout<< "   >>"<< m_disconnect_store[i].c_str()<< endl;
        CLogData::getInstance()->WriteLogFormat("disconnect", ">>", m_disconnect_store[i].c_str(), NULL);
    }
}

void* SocketServer::heartBeatThread(void* args) {
    SocketServer *pInstance = (SocketServer*)args;
    int rv = 0;
    
    while(true) {
        sleep(3);
        pInstance->sendHeartbeat();
 //       pInstance->showDisConnectStore();
    }
    pthread_exit(&rv);
}

bool SocketServer::runHeartBeatThread() {
    int rv = 0;
    rv = pthread_create(&m_heartbeat_thread_id, NULL, heartBeatThread, this);
    if(rv != 0) {
        cout<< "heartBeatThread failed"<< endl;
        return false;
    }
    return true;
}
