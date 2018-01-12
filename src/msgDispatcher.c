#include <stdio.h>
#include <pthread.h>
#define BUFFER_SIZE 16 // ����������
#include "circlebuff.h"

extern unsigned char AP_PacketBuff[];
extern DATAS_BUFF_T   comBuff0; 
void msgDisPatcherTreat(void);

void *msgDisPatcherThread(void)
 {
	 
	 while (1)
	 {
		 pthread_mutex_lock(&comBuff0.lock);
		 pthread_cond_wait(&comBuff0.newPacketFlag, &comBuff0.lock);
		 msgDisPatcherTreat();
		 pthread_mutex_unlock(&comBuff0.lock);
	 }
	 return NULL;
 }
 void msgDisPatcherTreat(void)
 {
	INT16U port;
	INT16U i;

	port = AP_circleBuff_ReadPacketData();
//	switch(port)
//	{
//		case MQTPA2DTU:
//						printf("11111111111111111PORT=%x \n", port);
//						break;
//		case DTU2MQTPA:
//						printf("22222222222222222PORT=%x \n", port);
//						break;
//        default:
//
//						printf("err port=%x \n", port);
//						break;
//
//	}
//	for(i=0;i<12;i++)
//	{
//		printf("%d", AP_PacketBuff[i]);
//	}

 }
