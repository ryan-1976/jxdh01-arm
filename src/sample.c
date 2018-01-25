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
int fd_spi = 0;

#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include "spidev.h"

#define SPI_DEVICE "/dev/spidev0.0"
#define GPIO_DEVICE "/sys/class/gpio/gpio68/value"
//static const char *device = "/dev/spidev0.0";
static U08 mode;
static U08 bits = 8;
static unsigned long int speed = 50000;
unsigned char buf_me[1] = {0x55};
static U16 delay;
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

void spiTest(void);
static void transfer(int fd);

static void pabort(const char *s)
{
	perror(s);
	abort();
}
void *sampleData_treat(void)
{
	char cdtuBuf[2048];
	int i;
	static unsigned char j;


	j=0;

	spiTest();

	sleep(3);
	while(1)
	{

		//printf("---enter ---sampleData_treat----------\n");
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

void spiTest(void)
{
	int ret = 0;

	fd_spi = open(SPI_DEVICE, O_RDWR); /* 打开 SPI 总线的设备文件 */
	if (fd_spi < 0)
	{
		printf("can't open %s \n", SPI_DEVICE);
	}
    //mode = mode | SPI_MODE_1 | SPI_LSB_FIRST | SPI_LOOP;
	mode =0;
 /*
  * spi mode
  */
 ret = ioctl(fd_spi, SPI_IOC_WR_MODE, &mode);
 if (ret == -1)
	 printf("can't set spi mode");

 ret = ioctl(fd_spi, SPI_IOC_RD_MODE, &mode);
 if (ret == -1)
	 printf("can't get spi mode");

 /*
  * bits per word
  */
 ret = ioctl(fd_spi, SPI_IOC_WR_BITS_PER_WORD, &bits);
 if (ret == -1)
	 printf("can't set bits per word");

 ret = ioctl(fd_spi, SPI_IOC_RD_BITS_PER_WORD, &bits);
 if (ret == -1)
	 printf("can't get bits per word");

 /*
  * max speed hz
  */
 ret = ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
 if (ret == -1)
	 printf("can't set max speed hz");

 ret = ioctl(fd_spi, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
 if (ret == -1)
	 printf("can't get max speed hz");

 printf("spi mode: %d\n", mode);
 printf("bits per word: %d\n", bits);
 printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

 while(1){
            //write(fd_spi,buf_me,1);
            transfer(fd_spi);
            sleep(1);
            printf("transfer\n");
        }
 //transfer(fd_spi);
    close(fd_spi);
}


static void transfer(int fd)
{
	int ret;
	U08 tx[] = {
		0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
	};
	//U08 rx[ARRAY_SIZE(tx)] = {0,};
	U08 rx[1000] = {0,};
	struct spi_ioc_transfer tr_txrx[] = {
		{
                .tx_buf = (unsigned long)tx,
                .rx_buf = 0,
                .len = 5,
                .delay_usecs = delay,
                .speed_hz = speed,
                .bits_per_word = bits,
		},
		{
		.rx_buf = (unsigned long)rx,
		.len = 100,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
		}
	};

        ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tr_txrx[0]);
        if (ret == 1) {
                pabort("can't revieve spi message");
	}
        else
        {
        	printf("recieve: ok \n");
        }

	for (ret = 0; ret < tr_txrx[1].len; ret++) {
		printf("%d ", rx[ret]);
	}
	printf("\n");
}
