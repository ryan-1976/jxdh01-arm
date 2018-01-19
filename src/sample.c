#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sqlite3.h"
#include "circlebuff.h"
#include "sqlite3_task.h"

//extern unsigned char AP_PacketBuff[];
extern DATAS_BUFF_T   comBuff0; 
extern pthread_mutex_t sqlWriteBufferLock;
extern pthread_cond_t  sqlWritePacketFlag;
typedef  unsigned char  U08;
typedef  unsigned short U16;
typedef  signed char    I08;
typedef  signed short   I16;

void *sampleData_treat(void)
{
	char cdtuBuf[2048];
	int i;
	static unsigned char j;
	sqlite3 *db = NULL; //声明sqlite关键结构指针
	int result;

	j=0;
	sleep(3);
	while(1)
	{

		printf("---enter ---sampleData_treat----------\n");
		for(i=0;i<1024;i++){
			cdtuBuf[i]=j+1;
		}
		j++;
		pthread_mutex_lock(&comBuff0.lock);
		//AP_circleBuff_WritePacket(tblStrinName,1024,DTU2MQTPA);
		AP_circleBuff_WritePacket(cdtuBuf,1024,DTU2MQTPA);
		pthread_cond_signal(&comBuff0.newPacketFlag);
		pthread_mutex_unlock(&comBuff0.lock);

		pthread_mutex_lock(&sqlWriteBufferLock);
		write_sqliteFifo(cdtuBuf,1024,0xff);
		pthread_cond_signal(&sqlWritePacketFlag);
		pthread_mutex_unlock(&sqlWriteBufferLock);

		sleep(3);
	}
}
