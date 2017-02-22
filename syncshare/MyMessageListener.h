/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MyMessageListener.h
 * Author: tx
 *
 * Created on 2016年3月21日, 下午4:02
 */

#ifndef MYMESSAGELISTENER_H
#define MYMESSAGELISTENER_H
#include <mq/Message.h>
#include <mq/MessageListener.h>
using namespace ons;


class  MessageListenerErp: public  MessageListener {
public:
    MessageListenerErp();
    MessageListenerErp(const MessageListenerErp& orig);
    virtual ~MessageListenerErp();
    
    virtual Action consume(Message &message, ConsumeContext &context);
private:

};

class MessageListenerErpFinal: public MessageListener{
public:
    MessageListenerErpFinal();
    virtual ~MessageListenerErpFinal();
    virtual Action consume(Message &message, ConsumeContext &context);
private:
    
};

class MessageListenerException: public MessageListener{
public:
    MessageListenerException();
    virtual ~MessageListenerException();
    virtual Action consume(Message &message, ConsumeContext &context);
};
#endif /* MYMESSAGELISTENER_H */

