#include <stddef.h>
#include "sqlite3.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "sqlite3_task.h"
const char* file_database_path = "/home/ryan"; //文件数据库存放路径
sqlite3 *memDevSampleDb;

const char* sql_create_data = "CREATE TABLE devSampleInfo (id INTEGER PRIMARY KEY, devId INTEGER,devSampleId INTEGER, timestamp INTEGER,message BLOB);";
const char* sql_insert_data = "INSERT OR REPLACE INTO devSampleInfo VALUES('%d', '%d', %d, %d,?);";
const char* sql_delete_data = "DELETE FROM devSampleInfo WHERE id = '%d';"; //删除内存数据库，需同时删除内存
const char* sql_update_data = "UPDATE devSampleInfo SET message = '%s', offset = %d, timestamp = %d where id = '%s';";//更新数据库，需同时更新内存、文件数据库中的内容
const char* sql_search_data = "SELECT * FROM devSampleInfo WHERE timestamp BETWEEN %d AND %d union SELECT * FROM testdb.testinfo WHERE timestamp BETWEEN %d AND %d;"; //查找数据库，将内存、文件数据库中查找出的内容合并
const char* sql_transfer_data = "INSERT OR REPLACE INTO filedb.testinfo SELECT * FROM testinfo;";   //将内存数据库中的信息同步到文件数据库中
const char* sql_delete_memory_table = "DELETE FROM testinfo;";	//内存数据库中的内容同步结束后，清空

#define BUFSIZE	4096
volatile static unsigned int read_pos=0, write_pos=0;
volatile static unsigned char full=0, empty=1;
static unsigned char buffer[BUFSIZE];
extern pthread_mutex_t sqlWriteBufferLock;
extern pthread_cond_t  sqlWritePacketFlag;

int InsertMemDevRecord(unsigned int id, unsigned int  devId, unsigned int  devSampleId, unsigned int  timestamp,unsigned int  messagelen,const char* message)
{
    int      rc              =  0;
   // char*    errMsg          =  NULL;
    char     sqlcmd[512]     =  {0};
   // time_t   insertTimestamp =  0;
    sqlite3_stmt *stmt = NULL;

    snprintf(sqlcmd, sizeof(sqlcmd), sql_insert_data, id, devId, devSampleId, timestamp);
    rc = sqlite3_prepare(memDevSampleDb, sqlcmd, strlen(sqlcmd), &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "memDevSampleDb prepare fail, errcode[%d], errmsg[%s]\n", rc, sqlite3_errmsg(memDevSampleDb));
        sqlite3_close(memDevSampleDb);
        return -1;
    }

    rc = sqlite3_bind_blob(stmt, 1, &message, messagelen, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "memDevSampleDb bind fail, errcode[%d], errmsg[%s]\n", rc, sqlite3_errmsg(memDevSampleDb));
        sqlite3_close(memDevSampleDb);
        return -1;
    }
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "memDevSampleDb insert fail, errcode[%d], errmsg[%s]\n", rc, sqlite3_errmsg(memDevSampleDb));
        sqlite3_close(memDevSampleDb);
        return -1;
    }

    sqlite3_finalize(stmt);
    printf("------------write db ok -------------------------------------");
    return 0;
}


//创建文件数据库
int CreateDbOnFile()
{
    sqlite3 *db           = NULL;
    int      rc           = 0;
    char*    errMsg       = NULL;
    char     sqlcmd[512]  = {0};


    rc = sqlite3_open(file_database_path, &db);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "cat't open database:%s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    
	snprintf(sqlcmd, sizeof(sqlcmd), sql_create_data);
	rc = sqlite3_exec(db, sqlcmd, NULL, NULL, &errMsg);
	if (SQLITE_OK != rc) {
		fprintf(stderr, "cat't create file database testinfo:%s\n", errMsg);
		sqlite3_close(db);
		return -1;
	}

    sqlite3_close(db);
    return 0;
}

//创建内存数据库
int CreateDbOnMemery()
{
    int      rc           = 0;
    char*    errMsg       = NULL;
    char     sqlcmd[512]  = {0};

    rc = sqlite3_open(":memory:", &memDevSampleDb);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "cat't open database:%s\n", sqlite3_errmsg(memDevSampleDb));
        sqlite3_close(memDevSampleDb);
        return -1;
    }
    
	snprintf(sqlcmd, sizeof(sqlcmd), sql_create_data);
	rc = sqlite3_exec(memDevSampleDb, sqlcmd, NULL, NULL, &errMsg);
	if (SQLITE_OK != rc) {
		fprintf(stderr, "cat't create memory database %s\n", errMsg);
		sqlite3_close(memDevSampleDb);
		return -1;
	}

    return 0;
}

//初始化数据库，分别创建文件数据库、内存数据库并把文件数据库attach到内存数据库上
int InitSqliteDb()
{
    int retval = 0;

//    retval =  CreateDbOnFile();
//    if (retval != 0) {
//        return retval;
//    }

    retval =  CreateDbOnMemery();
    if (retval != 0) {
        return retval;
    }

//    retval =  AttachDb();
//    if (retval != 0) {
//        return retval;
//    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////

//参数说明:

//pInMemory: 指向内存数据库指针

//zFilename: 指向文件数据库目录的字符串指针

//isSave  0: 从文件数据库载入到内存数据库 1：从内存数据库备份到文件数据库

////////////////////////////////////////////////////////////////////////////////////////////

int loadOrSaveDb(sqlite3 *pInMemeory, const char *zFilename, int isSave)
{
         int rc;
         sqlite3 *pFile;
         sqlite3_backup *pBackup;
         sqlite3 *pTo;
         sqlite3 *pFrom;

         rc = sqlite3_open(zFilename, &pFile);

         if(rc == SQLITE_OK)
         {
			   pFrom = (isSave?pInMemeory:pFile);
			   pTo = (isSave?pFile:pInMemeory);
			   pBackup = sqlite3_backup_init(pTo,"main",pFrom,"main");

			   if(pBackup)
			   {
						(void)sqlite3_backup_step(pBackup,-1);
						(void)sqlite3_backup_finish(pBackup);
			   }
				rc = sqlite3_errcode(pTo);
         }
         (void)sqlite3_close(pFile);

         return rc;
}

void *sqlite_treat(void)
{
	int i;
	unsigned char readFifoData[2048]  = {0xff};
	printf("---enter ---sqlite_treat----------\n");
	InitSqliteDb();
	static unsigned int id=0,devId=0x55,devSampleId=0,timestamp=0x88;

	while(1)
	{
		pthread_mutex_lock(&sqlWriteBufferLock);
		pthread_cond_wait(&sqlWritePacketFlag, &sqlWriteBufferLock);
		printf("---enter ---sqlite_data_treat----------\n");
		read_sqliteFifo(readFifoData);
		id ++;
		devSampleId++;
		InsertMemDevRecord(id, devId, devSampleId, timestamp,1027,readFifoData);
//		printf("sqlite recvieve data:\n");
//        for(i=0;i<1027;i++)
//        {
//        	printf("%d",readFifoData[i]);
//        }
//        printf("\n");
		//msgDisPatcherTreat();
        //sleep(10);
		pthread_mutex_unlock(&sqlWriteBufferLock);
	}
	return NULL;
}



void write_byte(unsigned char *src)
{
	//should disable RE when operating at system level

	if (!full){
	buffer[write_pos] = *src;
	write_pos = (write_pos + 1)%BUFSIZE;
	if (write_pos == read_pos)
		full = 1;
	empty = 0;
	}

}

void read_byte(unsigned char *dst)
{
	if(!empty){
	*dst = buffer[read_pos];
	read_pos = (read_pos + 1)%BUFSIZE;
	if(read_pos == write_pos)
		empty = 1;
	full = 0;
	}
}

void write_sqliteFifo(unsigned char *wbuf, unsigned int len,unsigned char type)
{
	unsigned int i;
	unsigned char tmp[3];

	tmp[0]=(unsigned char) (len>>8);
	tmp[1]=(unsigned char) len;
	tmp[2]= type;

	write_byte(&tmp[0]);
	write_byte(&tmp[1]);
	write_byte(&tmp[2]);

	for( i=0;(i<len)&&(!full);i++)write_byte(wbuf++);
	return;
}

void read_sqliteFifo(unsigned char *rbuf)
{
	int i;
	int len;

	unsigned char tmp[2];

	read_byte(&tmp[0]);
	read_byte(&tmp[1]);
	len =tmp[0]*256+tmp[1];

	*rbuf++ = tmp[0];
	*rbuf++ = tmp[1];
	for(i=0; (i<len+1)&&(!empty);i++){
		read_byte(rbuf++);
	}
	return;
}
