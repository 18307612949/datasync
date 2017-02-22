/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   socket_message.h
 * Author: tx
 *
 * Created on 2016年4月15日, 下午3:51
 */

#ifndef SOCKET_MESSAGE_H
#define SOCKET_MESSAGE_H
#include <iostream>
using namespace std;
#define MSG_TYPE_STORECODE 101
#define MSG_TYPE_BINLOG    102
#define MSG_TYPE_UPDATE_DATABASE 103
#define MSG_TYPE_HEART_BEAT     104
#define MSG_TYPE_CONNECT_SUCCESS 105
typedef struct socketmessage{
	int code_;
	int data_len_;
	char message_id_[33];
//	string data_;
}SOCKET_MESSAGE_HEADER;
#endif /* SOCKET_MESSAGE_H */
