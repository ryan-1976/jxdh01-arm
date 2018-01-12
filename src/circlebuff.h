/*
*******************************************************************************
*                    Copyright (c) 2013, TQ System
*                             All Rights Reserved
*
* Purpose           : COM file
* File Name         : T_ap_circlebuff.c

* Revision history  :
    01a,2013/07/29,Ryan  -- Creation(written)

* Description       :

*******************************************************************************
*/
#include <pthread.h>

#ifndef _AP_CIRCLEBUFF_INCLUDED_
#define _AP_CIRCLEBUFF_INCLUDED_

//#define   TZigbee_PACK_HEAD       0xA5
#define   MAX_BUFF_LEN            102400
#define   MQ_SENT_BUFF_LEN        2024
#define   RECV_BUFF_LEN           2024
#define   MAX_PACKET_BUFF_LEN     2100  
#define BUSY 1;
#define IDLE 0;

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                  
typedef signed   char  INT8S;                   
typedef unsigned short INT16U;                  
typedef signed   short INT16S;                 
typedef unsigned int   INT32U;                  
typedef signed   int   INT32S;                 

#define CDTU       0X01
#define MQTT       0X02
#define UDP        0X03
#define MODEM      0X04
#define SERIAL1    0X05

#define MQTPA2DTU    0X0201
#define DTU2MQTPA    0X0102
#define MQTPB2DTU    0X0301
#define DTU2MQTPB    0X0103
#define MQTPA        0X00
#define MQTPB       0X01

typedef struct
{
    unsigned long int    readPos;
    unsigned long int    writePos;
    unsigned char   data[MAX_BUFF_LEN];
	pthread_mutex_t lock;
	pthread_cond_t  newPacketFlag;
	
}DATAS_BUFF_T;
typedef struct
{
    unsigned char   data[MQ_SENT_BUFF_LEN];
    unsigned char   mqttTopicFlag;
    unsigned int    len;
	pthread_mutex_t lock;
	pthread_cond_t  newPacketFlag;
}MQTT_SENT_BUFF_T;
typedef struct
{
    unsigned char   data[RECV_BUFF_LEN];
    unsigned char   protoltype;
    unsigned char   scrFlag;//data from where
    unsigned int    len;
	pthread_mutex_t lock;
	pthread_cond_t  newPacketFlag;
}RECV_BUFF_T;

void AP_circleBuff_comBuff0_init(void);
unsigned char AP_circleBuff_HaveData_Buff(void);
unsigned char AP_circleBuff_WriteData(unsigned char data);
unsigned char AP_circleBuff_ReadData(void);
INT16U AP_circleBuff_ReadPacketData(void);

void AP_circleBuff_WritePacket(unsigned char *s, INT16U len, INT16U port);

#endif

