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

#define hang 1000
#define POLLPACKETMAXLEN 100


typedef struct{
	U08  paraGroupGrade;
	U08  groupNum;
	U08  groupAddrLen;
	U16  dynamicLenId;
	U08  pollFreq;
    U08  paraValueType;
    U16  mbAddr;
	U16  paraHexId;
	U16  paraId;
    U08  paraLen;
	U16  *paraName;
	U08  radio;
	U08  paraUnion;
	U08  mainGuiOp;
	U08  guiOp1;
	U08  guiOp2;
	U08  dataTreatType;
	U08  right;
	U08  limitOp;
	U16  limitDnValue;
	U16  limitUpValue;
	U16  *enumValue;
}TBL_COMM;


typedef struct{
	U16  tabId;
	U08  paraProperty1;
	U08  paraProperty2;
	U16  paraId;
    U08  paraLen;
    U08  paraValueType;
    U08  pollFreq;
    U16  mbAddr;
    U08  longParaIndex;
    U08  comPacketIndex;
	U08  byteIndex;
	U08  bitIndex;
	U08  dev_value[2];
	U08  server_value[2];
}TBL_CONTROL;

typedef struct{
	U08  cmd;
	U16  starMbAddr;
	U16  byteSum;
}CONTENT_RECORD;

typedef struct{
	U16   			packetInex;
	U16             starAddrIndex;
	CONTENT_RECORD  content;
}PACKET_record;

typedef struct{
	U08    dev_value[50];
	U08    server_value[50];
}STRING_RECORD;

STRING_RECORD StringPara[40];
PACKET_record CommPacket[100];
U08 MbComRecData[255];

const U08 mask[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
int g_comId =0;

U08 Type_00_sum = 3;
U08 Type_01_sum = 2;
U08 Type_02_sum = 5;
U08 Type_03_sum = 7;

U16 g_maxLen;
U16 ComPacketIndex =0;
U16 ControlMaxLen;

U08 tblStrinName[30000];
U08 tblStrinEnumName[1000];
U08 tblStrinUnionName[1000];
TBL_COMM    tbl_comm[hang];
TBL_CONTROL tbl_control[hang];
U08 CurrentComPacketIndex;
void SortByParaValueType(TBL_CONTROL *input,U16 n);
void SameTypeSortByMbAddr(TBL_CONTROL *input,U08 type);
void PollBits_PacketTreat(TBL_CONTROL *input, U08 cmd,U08 type,U08 mbAddrContinuFlag);
void PollWords_PacketTreat(TBL_CONTROL *input,U08 cmd,U08 type,U08 mbAddrContinuFlag);
void PollString_PacketTreat(TBL_CONTROL *input,U08 cmd,U08 type,U08 mbAddrContinuFlag);
void MbComRespPacketTreat(U08 *input);


//char * strcpy(char * strDest,const char * strSrc)
//{
//char * strDestCopy = strDest;
//while ((*strDest++=*strSrc++)!='\0');
//return strDestCopy;
//}
void Test()
{
	int i;

	CurrentComPacketIndex = 1;
	for(i=0;i<255; i++){
		MbComRecData[i] =i;
	}
	MbComRespPacketTreat(MbComRecData);

}

void FormControllerDataTab(TBL_COMM *input)
{
	TBL_COMM *pPtr;
    
    U16 i,j,longParaIndex;
    U16 m,n;
    U08 temp;
    
	pPtr =input;
    //----系统级别为0的参数处理------------------------------------------------------
    longParaIndex=0;
	for(i=0,j=0;i<g_maxLen;i++){
		if(pPtr[i].groupNum == 0 && pPtr[i].groupAddrLen == 0 ){
			tbl_control[j].tabId         =j;
			tbl_control[j].paraProperty1 =0;
			tbl_control[j].paraProperty2 =0;
			tbl_control[j].paraId        =pPtr[i].paraId;
			tbl_control[j].paraLen       =pPtr[i].paraLen;
			tbl_control[j].paraValueType =pPtr[i].paraValueType;
			tbl_control[j].mbAddr        =pPtr[i].mbAddr;
			tbl_control[j].pollFreq      =pPtr[i].pollFreq;
			tbl_control[j].longParaIndex = 0;
			
			if(pPtr[i].paraLen>2) {
				tbl_control[j].longParaIndex  =longParaIndex++;
			}
			j++;
		}
	}
	//-------系统级别为1的参数处理--------------------------------------------------
    temp =1;//临时记录最大的组序号值。
	for(i=0;i<g_maxLen;i++){
	  	if(pPtr[i].paraGroupGrade == 1){
	  		if(pPtr[i].groupNum >temp)temp=pPtr[i].groupNum;
	  	}
	}
	
	for(m=1;m<(temp+1);m++){
		for(i=0;i<g_maxLen;i++){
			if(pPtr[i].paraGroupGrade == 1 && pPtr[i].groupNum == m){
				//printf(" i =%d;  pPtr[i].dynamicLenId=%d\n ",i,pPtr[i].dynamicLenId);

				for(n=0;n<GetDynamicLen(pPtr[i].dynamicLenId);n++){
					
					tbl_control[j].tabId         =j;
					tbl_control[j].paraProperty1 =1;
					tbl_control[j].paraProperty2 =n;
					tbl_control[j].paraId        =pPtr[i].paraId;
					tbl_control[j].paraLen       =pPtr[i].paraLen;
					tbl_control[j].paraValueType =pPtr[i].paraValueType;
					tbl_control[j].pollFreq      =pPtr[i].pollFreq;
					tbl_control[j].mbAddr        =pPtr[i].mbAddr+pPtr[i].groupAddrLen*n;
					tbl_control[j].longParaIndex = 0;
					if(pPtr[i].paraLen>2) {
						tbl_control[j].longParaIndex  =longParaIndex++;
					}
					j++;
				}
			}
		}
	}
	  	//-------系统级别为2的参数处理--------------------------------------------------
    temp =0;//临时记录最大的组序号值。
	for(i=0;i<g_maxLen;i++){
	  	if(pPtr[i].paraGroupGrade == 2){
	  		if(pPtr[i].groupNum >temp)temp=pPtr[i].groupNum;
	  	}
	}
	
	for(m=1;m<(temp+1);m++){
		for(i=0;i<g_maxLen;i++){
			if(pPtr[i].paraGroupGrade == 2 && pPtr[i].groupNum == m){
				for(n=0;n<GetDynamicLen(pPtr[i].dynamicLenId);n++){
					tbl_control[j].tabId         =j;
					tbl_control[j].paraProperty1 =2;
					tbl_control[j].paraProperty2 =n;
					tbl_control[j].paraId        =pPtr[i].paraId;
					tbl_control[j].paraLen       =pPtr[i].paraLen;
					tbl_control[j].paraValueType =pPtr[i].paraValueType;
					tbl_control[j].pollFreq      =pPtr[i].pollFreq;
					tbl_control[j].mbAddr        =pPtr[i].mbAddr+pPtr[i].groupAddrLen*n;
					tbl_control[j].longParaIndex = 0;
					if(pPtr[i].paraLen>2) {
						tbl_control[j].longParaIndex  =longParaIndex++;
					}
					j++;
				}
			}
		}
	}
	//-----------------------------------------------------------------------------

    ControlMaxLen =j;
	// for(i=0;i<j;i++){
	// 	printf(" tabId=%d  paraProperty1=%d ",    tbl_control[i].tabId,tbl_control[i].paraProperty1);
	// 	printf(" paraProperty2=%d  paraId=%d ",tbl_control[i].paraProperty2,  tbl_control[i].paraId);
	// 	printf(" paraLen=%d   paraValueType=%d ",tbl_control[i].paraLen,  tbl_control[i].paraValueType);
	// 	printf(" mbAddr=%d   longParaIndex=%d ",tbl_control[i].mbAddr,  tbl_control[i].longParaIndex);
	// 	printf(" pollFreq=%d \n ",tbl_control[i].pollFreq);

	// }
}
int GetToken(char *pszSrc, char *pszSplit, char *pszDest)
{
    int iPos;
    char *pPtr = NULL;

    pPtr = strstr(pszSrc, pszSplit);
    if( pPtr ) 
    {
        iPos = strlen(pszSrc) - strlen(pPtr);
        strncpy(pszDest, pszSrc, iPos);
		memmove(pszSrc, pPtr+strlen(pszSplit),strlen(pPtr));
        pszDest[iPos] = '\0';
    }
    else
    {
        iPos = strlen(pszSrc);
        strncpy(pszDest, pszSrc, iPos);
        pszDest[iPos] = '\0';
        pszSrc[0] = '\0';
        return 0;
    }
    return 1;
}

int GetDynamicLen(int dynamicLenId)//临时处理函数，实际是MCU
{
	if(dynamicLenId == 4)return Type_00_sum;
	else if(dynamicLenId == 30)return Type_01_sum;
    else if(dynamicLenId == 21)return Type_02_sum;
    else if(dynamicLenId == 22)return Type_03_sum;
    else{
    	return 0;
    }
}
int tabProc(void)
{
	U16 i,j,temp,n;
	int rt;
	FILE *fid; 
	U08 str[hang][200];
	U08 szFiled1[hang][23][50];
	U16 paraNamelenIndx =2;
	U16 enumLenIndx =2;
	U16 unionLenIndx =2;

	//U16 fd;
    //fd=open("/home/tab3.csv","r");
	// fd = open("/dev/port" , O_RDWR|O_NONBLOCK);
	// if(fd < 0)
	// {
	// 	printf("can not open /sys/class/gpio/gpio14\n");
	// 	return -1;
	// }
	// printf(" open /sys/class/gpio/gpio14 ok!!!!\n");
 //   fid=fopen("/home/tab3.csv","r");
//-------------------------------------------------------------------------------------------------------------
    
	fid=fopen("../file/tab.csv","r");
	if(fid == NULL)
	{
		printf(" not exit tab.csv  \n");
		exit(0);
	}
  //rt=fscanf(fid,"%s",&str[0][0]);//去除第一行
	for(i=0; i<hang; i++){
		//memset(&str[i][0],0,50);
		rt=fscanf(fid,"%s",&str[i][0]);
		//if(i==0)continue;
		//printf(" %s\n",&str[i][0]);
		if(rt!= -1){
				for(n=0;n<22;n++)//共22列
			{
				GetToken((char *)&str[i][0], ",", (char *)&szFiled1[i][n][0]);
			}
					
		    n=0;
			
			tbl_comm[i].paraGroupGrade=atoi((const char *)&szFiled1[i][n++][0]);
		    tbl_comm[i].groupNum=atoi((const char *)&szFiled1[i][n++][0]);
		    tbl_comm[i].groupAddrLen=atoi((const char *)&szFiled1[i][n++][0]);
		    tbl_comm[i].dynamicLenId=atoi((const char *)&szFiled1[i][n++][0]);
		    tbl_comm[i].pollFreq=atoi((const char *)&szFiled1[i][n++][0]);
		    tbl_comm[i].paraValueType=atoi((const char *)&szFiled1[i][n++][0]);
		    tbl_comm[i].mbAddr=atoi((const char *)&szFiled1[i][n++][0]);
			//tbl_comm[i].paraHexId=atoi((const char *)&szFiled1[i][n++][0]);
			n++;
			tbl_comm[i].paraId=atoi((const char *)&szFiled1[i][n++][0]);
			// printf("tbl_comm[i].paraId=%d ;tbl_comm[i].mbAddr=%d",tbl_comm[i].paraId,tbl_comm[i].mbAddr);
			// printf(" \n");
			tbl_comm[i].paraLen=atoi((const char *)&szFiled1[i][n++][0]);
			//-----监控量名词处理-------------------------------------------------
			tblStrinName[paraNamelenIndx++] = strlen(&szFiled1[i][n][0])+3;
			tblStrinName[paraNamelenIndx++] = tbl_comm[i].paraId >>8;
			tblStrinName[paraNamelenIndx++] = tbl_comm[i].paraId ;
			strcpy(&tblStrinName[paraNamelenIndx],&szFiled1[i][n][0]);
			paraNamelenIndx +=strlen(&szFiled1[i][n++][0]);
			//------------------------------------------------------
			tbl_comm[i].radio=atoi((const char *)&szFiled1[i][n++][0]);
			//------监控量单位处理----------------------------------
			if(szFiled1[i][n][0] !=0X2F)
			{
				tblStrinUnionName[unionLenIndx++] = strlen(&szFiled1[i][n][0])+3;
				//printf(" len =%x",tblStrinUnionName[unionLenIndx-1]);
				tblStrinUnionName[unionLenIndx++] = tbl_comm[i].paraId >>8;
				tblStrinUnionName[unionLenIndx++] = tbl_comm[i].paraId ;
				// printf(" OIDH =%x",tblStrinUnionName[unionLenIndx-2]);
				// printf(" OIDL =%x",tblStrinUnionName[unionLenIndx-1]);
				
				strcpy(&tblStrinUnionName[unionLenIndx],&szFiled1[i][n][0]);
				// printf("STRING =%s \n",&szFiled1[i][n][0]);
				unionLenIndx +=strlen(&szFiled1[i][n++][0]);
			}
			else
			{
				n++;
			}
			//------------------------------------------------------
			tbl_comm[i].mainGuiOp=atoi((const char *)&szFiled1[i][n++][0]);
			tbl_comm[i].guiOp1=atoi((const char *)&szFiled1[i][n++][0]);
			tbl_comm[i].guiOp2=atoi((const char *)&szFiled1[i][n++][0]);
			tbl_comm[i].dataTreatType=atoi((const char *)&szFiled1[i][n++][0]);
			tbl_comm[i].right=atoi((const char *)&szFiled1[i][n++][0]);
			tbl_comm[i].limitOp=atoi((const char *)&szFiled1[i][n++][0]);
			tbl_comm[i].limitDnValue=atoi((const char *)&szFiled1[i][n++][0]);
			tbl_comm[i].limitUpValue=atoi((const char *)&szFiled1[i][n++][0]);
			// printf("tbl_comm[i].dataTreatType=%d ;tbl_comm[i].limitUpValue=%d",tbl_comm[i].dataTreatType,tbl_comm[i].limitUpValue);
			// printf(" \n");
			//------------------------------------------------------
			if(szFiled1[i][n][0] !=0X2F)
			{
				tblStrinEnumName[enumLenIndx++] = strlen(&szFiled1[i][n][0])+3;
				tblStrinEnumName[enumLenIndx++] = tbl_comm[i].paraId >>8;
				tblStrinEnumName[enumLenIndx++] = tbl_comm[i].paraId ;
				strcpy(&tblStrinEnumName[enumLenIndx],&szFiled1[i][n][0]);
				enumLenIndx +=strlen(&szFiled1[i][n++][0]);
			}
			else
			{
				n++;
			}
			
			// printf(" 777777777777777777--------------------------------------\n");
			// strcpy(&tbl_comm[i].name[0],&szFiled1[i][9][0]);
			// strcpy(&tbl_comm[i].enumName[0],&szFiled1[i][10][0]);
			
			// for(j=0;j<30;j++)
			// {
				// printf(" %x",tbl_comm[i].name[j]);
			// }
			
			 // printf("\n");
			 // for(j=0;j<30;j++)
			// {
				// printf(" %x",tbl_comm[i].enumName[j]);
			// }
			// printf("\n");
	        // printf(" 88888888888888888--------------------------------------\n");
		}
		else{
			g_maxLen =i;
			tblStrinName[0] = paraNamelenIndx >>8;
			tblStrinName[1] = paraNamelenIndx ;
			tblStrinEnumName[0] = enumLenIndx >>8;
			tblStrinEnumName[1] = enumLenIndx ;
			tblStrinUnionName[0] = unionLenIndx >>8;
			tblStrinUnionName[1] = unionLenIndx ;
			
			tblStrinName[paraNamelenIndx++]=0;
			tblStrinEnumName[enumLenIndx++]=0;
			tblStrinUnionName[unionLenIndx++]=0;
			//-------------------------------------------------
			// printf(" ------tblStrinUnionName-------------------------------\n");
			// for(i=2;i<200;i++){
				 // printf("%x",tblStrinUnionName[i]);
			// }
			// printf(" ------tblStrinEnumName-------------------------------\n");
			
			// for(i=2;i<200;i++){
				 // printf("%x",tblStrinEnumName[i]);
			// }
			// printf(" ------tblStrinName-------------------------------\n");
			// for(i=2;i<200;i++){
				 // printf("%x",tblStrinName[i]);
			// }
			// printf(" 3333--------------------------------------\n");
			// exit(0);
			//-------------------------------------------------
			break;
		}
		
	}
	fclose(fid);
//-------形成数据总表处理------------------------------------------------------------------------------------------------------
FormControllerDataTab(tbl_comm);
SortByParaValueType(tbl_control,ControlMaxLen);

SameTypeSortByMbAddr(tbl_control,1);
SameTypeSortByMbAddr(tbl_control,2);
SameTypeSortByMbAddr(tbl_control,4);
SameTypeSortByMbAddr(tbl_control,6);
for(i=0;i<100;i++){
	CommPacket[i].packetInex =0xff;
}
for(i=0;i<hang;i++){
	tbl_control[i].tabId =i;
}
PollBits_PacketTreat(tbl_control,2,2,0);
PollWords_PacketTreat(tbl_control,4,4,0);
PollWords_PacketTreat(tbl_control,3,6,0);	
PollString_PacketTreat(tbl_control,4,5,0);	
// ReadRObits_PacketTreat(tbl_control,2);
// ReadROwords_PacketTreat(tbl_control,4);
Test();
// for(i=0;i<ControlMaxLen;i++){

		// printf(" Pty1=%u ",    tbl_control[i].paraProperty1);
		// printf(" pty2=%u  paraId=%u ",tbl_control[i].paraProperty2,  tbl_control[i].paraId);
		// printf(" paraLen=%u ",tbl_control[i].paraLen);
		// printf(" longPIndx=%u pVType=%u  mbAddr=%u    ",tbl_control[i].longParaIndex,tbl_control[i].paraValueType,tbl_control[i].mbAddr);
		// printf(" Index=%u byteIndex=%u bitIndex=%u ",tbl_control[i].comPacketIndex,tbl_control[i].byteIndex,tbl_control[i].bitIndex);
		// printf(" dev_value[1]=%u dev_value[0]=%u \n",tbl_control[i].dev_value[1],tbl_control[i].dev_value[0]);

	// }
// for(i=0;i<100;i++){
	// if(CommPacket[i].packetInex ==0xff)break;
	// printf("index=%u ",CommPacket[i].packetInex);
	// printf("starAddrIndex=%u ",CommPacket[i].starAddrIndex);
	// printf("cmd=%u ",CommPacket[i].content.cmd);
	// printf("content.starMbAddr=%u--",CommPacket[i].content.starMbAddr);
	// printf("comNum=%u \n",CommPacket[i].content.byteSum);
// }
// for(i=0;i<40;i++){
	// printf("StringPara[0]=%u ",StringPara[i].dev_value[0]);
    // printf("StringPara[29]=%u \n ",StringPara[i].dev_value[29]);
// }
//-------------------------------------------------------------------------------------------------------------



	printf("------tabProc ok---\n");
    return 0;
}


void SortByParaValueType(TBL_CONTROL *input,U16 n)
{
	U16 i,j;
	TBL_CONTROL temp;

	for(i=0; i<n-1; i++){
		for(j=0; j<n-1-i; j++){
			if(input[j].paraValueType>input[j+1].paraValueType){
				temp=input[j];
				input[j]=input[j+1];
				input[j+1]=temp;
			}
		}
	}
}
void SameTypeSortByMbAddr(TBL_CONTROL *input,U08 type)
{
	U16 i,j,n,starAddr;
	TBL_CONTROL temp;
	TBL_CONTROL *pPtr;

	starAddr =0xffff;
	for(i=0,n=0; i<ControlMaxLen; i++){
		if(input[i].paraValueType == type){
			n++;
			if(starAddr ==0xffff)starAddr=i;
		}
	}
	if(starAddr ==0xffff) return;

    pPtr =input + starAddr;
	for(i=0; i<n-1; i++){
		for(j=0; j<n-1-i; j++){
			if(pPtr[j].mbAddr>pPtr[j+1].mbAddr){
				temp=pPtr[j];
				pPtr[j]=pPtr[j+1];
				pPtr[j+1]=temp;
			}
		}
	}
}
//BIT组包处理
void PollBits_PacketTreat(TBL_CONTROL *input, U08 cmd,U08 type,U08 mbAddrContinuFlag)
{
	U16 i,j,temp,n,starAddr;
	TBL_CONTROL *pPtr;
	U16 nextStarAddrIndex;

	starAddr =0xffff;
	for(i=0,n=0; i<ControlMaxLen; i++){
		if(input[i].paraValueType == type){
			n++;
			if(starAddr == 0xffff)starAddr=i;
		}
	}
	if(starAddr ==0xffff) return;
	pPtr =input + starAddr;
    
	nextStarAddrIndex=0;
	for(i=0;i<n;i++){
		CommPacket[ComPacketIndex].packetInex=ComPacketIndex;
		CommPacket[ComPacketIndex].starAddrIndex=starAddr+nextStarAddrIndex;
		CommPacket[ComPacketIndex].content.cmd=cmd;
		CommPacket[ComPacketIndex].content.starMbAddr=pPtr[nextStarAddrIndex].mbAddr;

		for(j=1;j<(POLLPACKETMAXLEN+1)*8;j++){
			pPtr[nextStarAddrIndex+j-1].comPacketIndex=ComPacketIndex;
			temp = pPtr[nextStarAddrIndex+j-1].mbAddr-pPtr[nextStarAddrIndex].mbAddr;
			pPtr[nextStarAddrIndex+j-1].byteIndex=temp/8;
			pPtr[nextStarAddrIndex+j-1].bitIndex =mask[temp%8];
			
			if((nextStarAddrIndex+j)>=n)break;
			else if((pPtr[nextStarAddrIndex+j].mbAddr-pPtr[nextStarAddrIndex].mbAddr)>=(POLLPACKETMAXLEN*8))break;
			else if((pPtr[nextStarAddrIndex+j].mbAddr-pPtr[nextStarAddrIndex].mbAddr)>=pPtr[n-1].mbAddr)break;
			else if(mbAddrContinuFlag == 1 && (pPtr[nextStarAddrIndex+j].mbAddr-pPtr[nextStarAddrIndex+j-1].mbAddr)!=1)break;
			else{};
		}
		CommPacket[ComPacketIndex].content.byteSum=pPtr[nextStarAddrIndex+j-1].mbAddr -pPtr[nextStarAddrIndex].mbAddr+1;
	    nextStarAddrIndex +=j;
	    ComPacketIndex++;
		if(nextStarAddrIndex>=n)break;
	}
}
void PollWords_PacketTreat(TBL_CONTROL *input,U08 cmd,U08 type,U08 mbAddrContinuFlag)
{
	U16 i,j,temp,n,starAddr;
	TBL_CONTROL *pPtr;
	U16 nextStarAddrIndex;

	starAddr =0xffff;
	for(i=0,n=0; i<ControlMaxLen; i++){
		if(input[i].paraValueType == type){
			n++;
			if(starAddr == 0xffff)starAddr=i;
		}
	}
	if(starAddr ==0xffff) return;
	pPtr =input + starAddr;

	nextStarAddrIndex=0;
	for(i=0;i<n;i++){
		CommPacket[ComPacketIndex].packetInex=ComPacketIndex;
		CommPacket[ComPacketIndex].starAddrIndex=starAddr+nextStarAddrIndex;
		CommPacket[ComPacketIndex].content.cmd=cmd;
		CommPacket[ComPacketIndex].content.starMbAddr=pPtr[nextStarAddrIndex].mbAddr;

		for(j=1;j<(POLLPACKETMAXLEN+1);j++){
			pPtr[nextStarAddrIndex+j-1].comPacketIndex=ComPacketIndex;
			temp = pPtr[nextStarAddrIndex+j-1].mbAddr-pPtr[nextStarAddrIndex].mbAddr;
			pPtr[nextStarAddrIndex+j-1].byteIndex=(U08) (temp*2);
			pPtr[nextStarAddrIndex+j-1].bitIndex =0xff;
			
			// printf("----nextStarAddrIndex=%u j=%u n=%u-------------\n",starAddr,j,n);
   //          printf("----pPtr[nextStarAddrIndex+j].mbAddr=%u -------------\n",pPtr[nextStarAddrIndex+j].mbAddr);
   //          printf("----pPtr[nextStarAddrIndex].mbAddr=%u -------------\n",pPtr[nextStarAddrIndex].mbAddr);
   //          printf("----pPtr[n].mbAddr=%u -------------\n",pPtr[n].mbAddr);
			if((nextStarAddrIndex+j)>=n)break;
			else if((pPtr[nextStarAddrIndex+j].mbAddr-pPtr[nextStarAddrIndex].mbAddr)>=POLLPACKETMAXLEN)break;
			else if((pPtr[nextStarAddrIndex+j].mbAddr-pPtr[nextStarAddrIndex].mbAddr)>=pPtr[n-1].mbAddr)break;
			else if(mbAddrContinuFlag == 1 && (pPtr[nextStarAddrIndex+j].mbAddr-pPtr[nextStarAddrIndex+j-1].mbAddr)!=1)break;
			else{};
		}
		CommPacket[ComPacketIndex].content.byteSum=pPtr[nextStarAddrIndex+j-1].mbAddr -pPtr[nextStarAddrIndex].mbAddr+1;
	    nextStarAddrIndex +=j;
	    ComPacketIndex++;
		if(nextStarAddrIndex>=n)break;
	}
}
void PollString_PacketTreat(TBL_CONTROL *input,U08 cmd,U08 type,U08 mbAddrContinuFlag)
{
	U16 i,j,temp,n,starAddr;
	TBL_CONTROL *pPtr;
	U16 nextStarAddrIndex;

	starAddr =0xffff;
	for(i=0,n=0; i<ControlMaxLen; i++){
		if(input[i].paraValueType == type){
			n++;
			if(starAddr == 0xffff)starAddr=i;
		}
	}
	if(starAddr ==0xffff) return;
	pPtr =input + starAddr;

	nextStarAddrIndex=0;
	for(i=0;i<n;i++){
		CommPacket[ComPacketIndex].packetInex=ComPacketIndex;
		CommPacket[ComPacketIndex].starAddrIndex=starAddr+nextStarAddrIndex;
		CommPacket[ComPacketIndex].content.cmd=cmd;
		CommPacket[ComPacketIndex].content.starMbAddr=pPtr[nextStarAddrIndex].mbAddr;

		for(j=1;j<(POLLPACKETMAXLEN+1);j++){
			pPtr[nextStarAddrIndex+j-1].comPacketIndex=ComPacketIndex;
			temp = pPtr[nextStarAddrIndex+j-1].mbAddr-pPtr[nextStarAddrIndex].mbAddr;
			pPtr[nextStarAddrIndex+j-1].byteIndex=(U08) (temp*2);
			pPtr[nextStarAddrIndex+j-1].bitIndex =0xff;

			temp = pPtr[nextStarAddrIndex+j-1].paraLen/2;
			if((nextStarAddrIndex+j)>=n)break;
			else if((pPtr[nextStarAddrIndex+j].mbAddr-pPtr[nextStarAddrIndex].mbAddr+pPtr[nextStarAddrIndex+j].paraLen/2)>POLLPACKETMAXLEN)break;
			else if((pPtr[nextStarAddrIndex+j].mbAddr-pPtr[nextStarAddrIndex].mbAddr)>(pPtr[n-1].mbAddr+temp*2))break;
			else if(mbAddrContinuFlag == 1 && (pPtr[nextStarAddrIndex+j].mbAddr-pPtr[nextStarAddrIndex+j-1].mbAddr)!=temp)break;
			else{};
		}
		CommPacket[ComPacketIndex].content.byteSum=pPtr[nextStarAddrIndex+j-1].mbAddr -pPtr[nextStarAddrIndex].mbAddr+pPtr[nextStarAddrIndex+j].paraLen/2;
	    nextStarAddrIndex +=j;
	    ComPacketIndex++;
		if(nextStarAddrIndex>=n)break;
	}
}

void MbComRespPacketTreat(U08 *input)
{
	U16 i,j,starAddr;
	U08 temp;
		temp = 0;
		starAddr = CommPacket[CurrentComPacketIndex].starAddrIndex;
		for(i=starAddr;i<ControlMaxLen; i++){
			if(tbl_control[i].comPacketIndex == CurrentComPacketIndex){
				if(tbl_control[i].bitIndex !=0xff){
					temp =input[tbl_control[i].byteIndex] & tbl_control[i].bitIndex ;
					if(temp !=0){
						tbl_control[i].dev_value[0] = 1;
					}
					else{
						tbl_control[i].dev_value[0] = 0;
					}
				}
				else if(tbl_control[i].paraLen <=2){
						tbl_control[i].dev_value[1] = input[tbl_control[i].byteIndex];
                        tbl_control[i].dev_value[0] = input[tbl_control[i].byteIndex +1];
				}
				else{
					for(j=0;j<tbl_control[i].paraLen; j++){
						StringPara[tbl_control[i].longParaIndex].dev_value[j]= input[tbl_control[i].byteIndex+j];
					}
				}
			}
			else{
				break;
			}
		}
}
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
