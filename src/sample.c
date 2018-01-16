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
extern unsigned char AP_PacketBuff[];
extern DATAS_BUFF_T   comBuff0; 
typedef  unsigned char  U08;
typedef  unsigned short U16;
typedef  signed char    I08;
typedef  signed short   I16;

void *cdtu_treat(void)
{
	char cdtuBuf[2048];
	int i;
	sqlite3 *db = NULL; //声明sqlite关键结构指针
	int result;

	    //打开数据库
	    //需要传入 db 这个指针的指针，因为 sqlite3_open 函数要为这个指针分配内存，还要让db指针指向这个内存区
	  result = sqlite3_open("Dcg_database.db", &db );
	    if( result != SQLITE_OK )
	    {
	        //数据库打开失败
	       return -1;
	    }
	while(1)
	{
		printf("---4-33enter ---cdtu_treat----------\n");
		for(i=0;i<1024;i++){
			cdtuBuf[i]=i;
		}
		pthread_mutex_lock(&comBuff0.lock);
		//AP_circleBuff_WritePacket(tblStrinName,1024,DTU2MQTPA);
		AP_circleBuff_WritePacket(cdtuBuf,1024,DTU2MQTPA);
		pthread_cond_signal(&comBuff0.newPacketFlag);
		pthread_mutex_unlock(&comBuff0.lock);
		sleep(3);
	}
}
