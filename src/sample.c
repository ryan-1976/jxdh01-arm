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

int fd_spi = 0;

#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include "spidev.h"
#include "spiCom.h"

static U08 mode;
static U08 bits = 8;
static unsigned long int speed = 500000;
unsigned char buf_me[1] = {0x55};
static U16 delay;
static U08 g_mcuPaketNumWait4Get;

U08 spiTxBuff[BUFFER_SIZE] = {0,};
U08 spiRxBuff[BUFFER_SIZE] = {0,};

extern void spiPacket(void);
U08  spiComPacketTreat(void);
static U08 spi_TxRx(int fd);
U08 spi_Init(void);
extern void spi2MqtttPacket(void);

static void pabort(const char *s)
{
	perror(s);
	abort();
}
void *sampleData_treat(void)
{
	//
	int i;

	U08 initFlag;
	U08 spiComFlag;

	initFlag=1;
	while(initFlag)
	{
		initFlag=spi_Init();
		sleep(1);
	}

	while(1)
	{
		//printf("---enter ---sampleData_treat----------\n");
		for(i=0;i<1024;i++){
			//cdtuBuf[i]=j+1;
		}
		spiComFlag= spi_TxRx(fd_spi);
		if(spiComFlag==0 &&spiComPacketTreat()==1)
		{
			printf("---enter ---nmamtf----------\n");
			pthread_mutex_lock(&comBuff0.lock);
			spi2MqtttPacket();
			//AP_circleBuff_WritePacket(cdtuBuf,BUFFER_SIZE,DTU2MQTPA);
			pthread_cond_signal(&comBuff0.newPacketFlag);
			pthread_mutex_unlock(&comBuff0.lock);

			pthread_mutex_lock(&sqlWriteBufferLock);
			//write_sqliteFifo(cdtuBuf,1024,0xff);
			pthread_cond_signal(&sqlWritePacketFlag);
			pthread_mutex_unlock(&sqlWriteBufferLock);

			if(g_mcuPaketNumWait4Get !=0)
				{
				printf("---enter ---300ms----------\n");
				usleep(250000);//delay 250ms
				}
			else {
				printf("---enter ---1s--ok--------\n");
				sleep(1);//delay 1s
			}
		}
		else
		{
			printf("---enter ---1s--err--------\n");
			sleep(1);//delay 1s
		}
	}
	close(fd_spi);
}
U08 sumCheck(void){
	U08 ret,sum;
	U16 i;

	ret=1;
	sum=0;
	for(i=1;i<BUFFER_SIZE-1;i++)
	{
		sum += spiRxBuff[i];
	}
	if(sum !=spiRxBuff[BUFFER_SIZE-1]){
		ret=0;
	}
	//ret=1;
	return ret;
}
U08 spiComPacketTreat(void)
{
	U08 ret;

	ret=0;
	if(spiRxBuff[0]==0xaa&&spiRxBuff[1]<=8&&sumCheck()==1)
	{
		ret=1;
		g_mcuPaketNumWait4Get= spiRxBuff[BUFFER_SIZE-2];
	}
	return ret;
}


U08 spi_Init(void)
{
	int ret = 0;
	U08 flag = 0;

	fd_spi = open(SPI_DEVICE, O_RDWR); /* 打开 SPI 总线的设备文件 */
	if (fd_spi < 0)
	{
		printf("can't open %s \n", SPI_DEVICE);
		flag =1;
	}
    //mode = mode | SPI_MODE_1 | SPI_LSB_FIRST | SPI_LOOP;
	mode =0;
 /*
  * spi mode
  */
 ret = ioctl(fd_spi, SPI_IOC_WR_MODE, &mode);
 if (ret == -1)
 {
	 printf("can't set spi mode");
	 flag = 1;
 }
 ret = ioctl(fd_spi, SPI_IOC_RD_MODE, &mode);
 if (ret == -1){
	 flag = 1;
	 printf("can't get spi mode");
 }
 /*
  * bits per word
  */
 ret = ioctl(fd_spi, SPI_IOC_WR_BITS_PER_WORD, &bits);
 if (ret == -1){
	 flag = 1;
	 printf("can't set bits per word");
 }

 ret = ioctl(fd_spi, SPI_IOC_RD_BITS_PER_WORD, &bits);
 if (ret == -1){
	 flag = 1;
	 printf("can't get bits per word");
 }


 /*
  * max speed hz
  */
 ret = ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
 if (ret == -1){
	 flag = 1;
	  printf("can't set max speed hz");
 }


 ret = ioctl(fd_spi, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
 if (ret == -1){
	 flag = 1;
	  printf("can't get max speed hz");
 }

return flag;
// printf("spi mode: %d\n", mode);
// printf("bits per word: %d\n", bits);
// printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
}

static U08 spi_TxRx(int fd)
{
	int ret;
	int i,m;
	static int j;
	j++;
	for (i = 0; i < BUFFER_SIZE; i++)
	{
		spiRxBuff[i]=0xff;
		spiTxBuff[i]=0x11;
	}
	spiTxBuff[0]=0xAA;
	spiTxBuff[1]=0x55;
	struct spi_ioc_transfer tr_txrx[] = {
		{
                .tx_buf = (unsigned long)spiTxBuff,
                .len = BUFFER_SIZE,
                .delay_usecs = delay,
                .speed_hz = speed,
                .bits_per_word = bits,
		},
		{
		.rx_buf = (unsigned long)spiRxBuff,
		.len = BUFFER_SIZE,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
		}
	};
        ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tr_txrx[0]);
        if (ret == 1)
		{
        	pabort("can't revieve spi message\\n");
		}
		else
		{
			printf("recieve: ok \n");
		}
        i=0;
		for (m = 0; m < 10; m++)
		{
			printf("%x ", spiRxBuff[i++]);
		}
		for (m = 0; m < 720; m++)
		{
			if(m%48==0)printf("\n");
			printf("%x ", spiRxBuff[i++]);
		}
		printf("\n");
		printf("%x ", spiRxBuff[BUFFER_SIZE-2]);
		printf("%x ", spiRxBuff[BUFFER_SIZE-1]);
		printf("\n");
		return ret;
}
