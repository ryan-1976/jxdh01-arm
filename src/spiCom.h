

#ifndef SPICOM_H
#define SPICOM_H

#include <linux/types.h>
typedef  unsigned char  U08;
typedef  unsigned short U16;
typedef  unsigned int   U32;
typedef  signed char    I08;
typedef  signed short   I16;

typedef struct{
	U08        startFlag;
	U08        packetLen[2];
	U08        packetNum[2];
	U08        siteId[4];
	U08        devId;
	U08        packetCmd;
	U08        packetAnswer;
}COM_HEAD_TYPE;

#define PACKET_HEAD_LEN (sizeof(COM_HEAD_TYPE))
#define PACKET_CONTEXT_LEN 732
#define PACKET_LEN (PACKET_HEAD_LEN+PACKET_CONTEXT_LEN+3)

typedef union
{
	U08             buffer[PACKET_LEN];
    COM_HEAD_TYPE   head;
} COM_TYPE;

typedef union
{
	U32   lword;
	U08   byte[4];
} UNION_4_BYTE;
typedef union
{
	U16   word;
	U08   byte[2];
} UNION_2_BYTE;
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define BUFFER_SIZE 768
#define SPI_DEVICE "/dev/spidev0.0"
#define GPIO_DEVICE "/sys/class/gpio/gpio68/value"

#endif /* SPICOM_H */
