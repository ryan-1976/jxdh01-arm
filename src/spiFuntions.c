#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include "circlebuff.h"
#include "spiCom.h"
U08 cdtuBuf[PACKET_LEN];
extern U08 spiRxBuff[];
U16 g_comPacketIdx=0;
static U32 g_siteId=0x01020304;

void spi2MqtttPacket(void)
{
	COM_TYPE *pCom;
	UNION_4_BYTE tmp4;
	UNION_2_BYTE tmp2;
	U16 i;

	if(spiRxBuff[1]==0)//
	{
		return;
	}
	pCom =(COM_TYPE *)&cdtuBuf[0];
	pCom->head.startFlag = 0x7e;
	tmp2.word =PACKET_LEN-2;
	pCom->head.packetLen[1] = tmp2.byte[0];
	pCom->head.packetLen[0] = tmp2.byte[1];
	tmp2.word =g_comPacketIdx;
	pCom->head.packetNum[1] = tmp2.byte[0];
	pCom->head.packetNum[0] = tmp2.byte[1];
	g_comPacketIdx++;
	tmp4.lword= g_siteId;
	pCom->head.siteId[3] = tmp4.byte[0];
	pCom->head.siteId[2] = tmp4.byte[1];
	pCom->head.siteId[1] = tmp4.byte[2];
	pCom->head.siteId[0] = tmp4.byte[3];
	pCom->head.devId =     spiRxBuff[1];
	pCom->head.packetCmd = 0x82;
	pCom->head.packetAnswer = 0xff;
	pCom->buffer[PACKET_HEAD_LEN]=0xff;
	pCom->buffer[PACKET_HEAD_LEN+1]=0x0f;
	pCom->buffer[PACKET_HEAD_LEN+2]=0x01;

    for(i=0;i<PACKET_CONTEXT_LEN;i++){
    	pCom->buffer[i+3+PACKET_HEAD_LEN]=spiRxBuff[i+1];
    }
    pCom->buffer[PACKET_LEN-1]=0x7e;
	AP_circleBuff_WritePacket(cdtuBuf,PACKET_LEN,DTU2MQTPA);

}
