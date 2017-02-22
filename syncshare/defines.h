/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   defines.h
 * Author: tx
 *
 * Created on 2016年3月8日, 下午4:07
 */

#ifndef DEFINES_H
#define DEFINES_H
#define SERVER_PORT 8888    //socket服务端口号
#define SERVER_LISTEN_NUM 10  //socket listen number
#define BUFFER_RECV_TEMP 512    //socket接收数据缓存buffer

#define BUFFER_LOG 1024*3 //日志记录缓存大小
#define MESSAGE_BUFFER_OVER "&over&" //一次socket通信结束标志
#define MESSAGE_BUFFER_OVER_LEN 6

#define CLIENT_RECV_TIMEOUT 8
#define CLIENT_SEND_TIMEOUT 8
#define HEARTBEAT_TIMEOUT 3

/*阿里云　ONS消息队列相关配置*/
#define ONS_ACCESS_KEY "0iUkPdUiWcgSsw26"
#define ONS_SECRET_KEY "85w2EneWAUHZsHVcQBUGztuKl2SA7E"

#define ONS_WMS_TOPIC "WMS_TOPIC"
#define ONS_WMS_PRODUCER "PID_wms_producer"
#define ONS_WMS_CONSUMER "CID_wms_consumer"
#define ONS_ERP_TOPIC "ERP_TOPIC"
#define ONS_ERP_PRODUCER "PID_erp_producer"
#define ONS_ERP_CONSUMER "CID_erp_consumer"
#define ONS_ERP_FINAL_TOPIC "ERP_FINAL_TOPIC"
#define ONS_ERP_FINAL_PRODUCER "PID_erp_final_consumer"
#define ONS_ERP_FINAL_CONSUMER "CID_erp_final_consumer"



typedef enum topic_enum{
    erp_final_topic,exception_topic,queue_raw_topic
}TOPIC_ENUM;
typedef enum queueInstance{
    wms_producer,erp_producer,wms_consumer,erp_consumer,erp_final_consumer,exception_thread1_producer, exception_thread1_consumer
}QUEUE_INSTANCE;



#endif /* DEFINES_H */

