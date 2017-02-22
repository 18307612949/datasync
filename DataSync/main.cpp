/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: tx
 *
 * Created on 2016年3月4日, 上午11:22
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <unistd.h>
#include <AliMessageQueue.h>
#include "SocketServer.h"
#include "CProcessMqData.h"
#include "LogError.h"
#include <comm.h>
#include <signal.h>
#include <MyException.h>
using namespace std;
using namespace ons;

/*
 * 
 */

void init_daemon(){
    int pid;
    pid = fork();
    if (pid > 0) {
        exit(0);
    }
    else if (pid < 0) {
        exit(1);
    }
    setsid(); 
    pid = fork();
    if(pid > 0) {
        exit(0);
    }
    else if(pid < 0) {
        exit(1);
    }
    
    for(int i = 0; i < NOFILE; i++)
        close(i);
}

int main(int argc, char** argv) {
//    init_daemon();
    initMutex();
    try{
        
        SocketServer *socketServerInstance = SocketServer::getInstance();
        socketServerInstance->run();
        socketServerInstance->runHeartBeatThread(); 
        g_instance_CProcessMqData.StartThread();
        g_instance_CProcessMqData.StartThreadException();
        sleep(1);
//       g_instance_CProcessMqData.StartThreadProduct();
        AliMessageQueue::getInstance()->runServer(erp_final_consumer);
        AliMessageQueue::getInstance()->runServer(exception_thread1_consumer);
        g_instance_CProcessMqData.StartThreadRaw();
        socketServerInstance->waitServerThread();
 
    }
    catch(ExceptionSocket &e){
         cout<< e.what()<< endl;
         CLogError::getInstance()->WriteErrorLog(-1, e.what());
     }
     catch(ExceptionMessageQueue &e) {
          cout<< e.what()<< endl;
          CLogError::getInstance()->WriteErrorLog(-1, e.what());
      }
      catch(ExceptionBase &e) {
          cout<< e.what()<< endl;
          CLogError::getInstance()->WriteErrorLog(-1, e.what());
       }
    destoryMutex();
    return 0;
}

