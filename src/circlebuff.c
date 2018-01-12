/*
*******************************************************************************
*                    Copyright (c) 2013, T System
*                             All Rights Reserved
*
* Purpose           : COM file
* File Name         : com_sp.c

* Chip type         :
* IDE               :

* Revision history  :
    01a,2013/07/29,Ryan  -- Creation(written)

* Description       :

*******************************************************************************
*/
#include <pthread.h>
#include "circlebuff.h"
#include <stdio.h>
#include <pthread.h>



DATAS_BUFF_T   comBuff0; 
MQTT_SENT_BUFF_T mqSentBuff;
RECV_BUFF_T RecvBuff4treat;
unsigned char AP_PacketBuff[MAX_PACKET_BUFF_LEN];//���嵥�����ݰ�

void G_Buff_init(void)
{
    comBuff0.readPos = 0;
    comBuff0.writePos = 0;
	pthread_mutex_init(&comBuff0.lock, NULL);
	pthread_cond_init(&comBuff0.newPacketFlag, NULL);
	pthread_mutex_init(&mqSentBuff.lock, NULL);
	pthread_cond_init(&mqSentBuff.newPacketFlag, NULL);
	pthread_mutex_init(&RecvBuff4treat.lock, NULL);
	pthread_cond_init(&RecvBuff4treat.newPacketFlag, NULL);
}
//�ж��Ƿ���������
unsigned char  AP_circleBuff_HaveData_Buff(void)
{
    if (comBuff0.readPos != comBuff0.writePos)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
//�ӻ�����д���ֽ����ݣ�
unsigned char  AP_circleBuff_WriteData(unsigned char data)
{
    unsigned long int  nextPos;
	//printf("\n");
	//printf("-----comBuff0.writePos= %d\n",comBuff0.writePos);
    nextPos = (comBuff0.writePos + 1) % MAX_BUFF_LEN;
    if (nextPos != comBuff0.readPos)    // �жϻ����Ƿ���
    {
        comBuff0.data[comBuff0.writePos] = data;
        comBuff0.writePos = nextPos;
		//printf("-----enter--1111111111111111111111111 \n");
        return 1;
		
    }
    else
    {
		//printf("-----enter--000000000000000000000 \n");
        return 0;
    }
}
//�ӻ����ж����ֽ����ݣ�
unsigned char AP_circleBuff_ReadData(void)
{
    unsigned char dataTemp =0;

    if (AP_circleBuff_HaveData_Buff() == 1)
    {
        dataTemp = comBuff0.data[comBuff0.readPos];
        comBuff0.readPos = (comBuff0.readPos + 1) % MAX_BUFF_LEN;
    }
    return  dataTemp;
}


INT16U AP_circleBuff_ReadPacketData(void)
{
    INT16U dataLen;
    INT16U port;
    INT16U i;
	// printf("\n");
	// printf("-----enter--AP_circleBuff_ReadPacketData-------------- \n");
	dataLen =0;
    if(AP_circleBuff_HaveData_Buff())
    {
		port=AP_circleBuff_ReadData();
		port =port *256;
		port +=AP_circleBuff_ReadData();
		
        dataLen=AP_circleBuff_ReadData();
		dataLen =dataLen *256;
		dataLen +=AP_circleBuff_ReadData();
	// printf("(port)= %d",port);
	// printf("(len)=----------------- %d",dataLen);
	// printf("\n");
    }
    else
    {
		// printf("---------------err---------------\n");
        return 0;
    }
	switch(port)
	{
		case MQTPA2DTU:
			            //printf("-enter-MQTPA2DTU-----------------");
						pthread_mutex_lock(&RecvBuff4treat.lock);
						for(i=0;i<dataLen;i++)
						{
							RecvBuff4treat.data[i]=AP_circleBuff_ReadData();
						}
						RecvBuff4treat.protoltype = 0;
						RecvBuff4treat.scrFlag = MQTPA;
						RecvBuff4treat.len = dataLen;
						//printf("--MQTPA2DTU--copy ok---------\n");
						pthread_cond_signal(&RecvBuff4treat.newPacketFlag);
						pthread_mutex_unlock(&RecvBuff4treat.lock);
						break;
		case DTU2MQTPA:
						//printf("----enter---DTU2MQTPA------------");
						pthread_mutex_lock(&mqSentBuff.lock);

						for(i=0;i<dataLen;i++)
						{
							mqSentBuff.data[i]=AP_circleBuff_ReadData();
						}
						mqSentBuff.mqttTopicFlag = MQTPA;
						mqSentBuff.len = dataLen;
						//printf("-----DTU2MUTPA copy ok--------------\n");
						pthread_cond_signal(&mqSentBuff.newPacketFlag);
						pthread_mutex_unlock(&mqSentBuff.lock);
						break;
        default:

						printf("err port=%x \n", port);
						break;

	}
//    for(i=0;i<dataLen;i++)
//    {
//        AP_PacketBuff[i]=AP_circleBuff_ReadData();
//		//printf("%d",AP_PacketBuff[i]);
//
//    }
	// printf("\n");
    return port;
}

void AP_circleBuff_WritePacket(INT8U *s, INT16U len, INT16U port)
{
    unsigned int i;
	
	// printf("\n");
	// printf("-----enter--AP_circleBuff_WritePacket-------------- \n");
	AP_circleBuff_WriteData((unsigned char) (port>>8));
	AP_circleBuff_WriteData((unsigned char)port);
	
	// printf("port=----------------- %d\n",port);
    AP_circleBuff_WriteData((unsigned char) (len>>8));
	AP_circleBuff_WriteData((unsigned char)len);
	// printf("(len_h)=----------------- %d",(len>>8));
	// printf("(len_l)=----------------- %d\n",len);
    for (i=0; i<len; i++)
    {
        AP_circleBuff_WriteData(s[i]);
		// printf("%d",s[i]);
    }
	// printf("\n");
}

