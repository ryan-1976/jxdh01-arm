const char* file_database_path = "/home/ryan"; //文件数据库存放路径

const char* sql_create_data = "CREATE TABLE testinfo (id TEXT PRIMARY KEY, message TEXT, offset INTEGER, timestamp INTEGER);";
const char* sql_insert_data = "INSERT OR REPLACE INTO MAIN.testinfo VALUES('%s', '%s', %d, %d);";
const char* sql_delete_data = "DELETE FROM MAIN.testinfo WHERE id = '%s'; DELETE FROM filedb.testinfo WHERE id = '%s';"; //删除数据库，需同时删除内存、文件数据库中的内容
const char* sql_update_data = "UPDATE MAIN.testinfo SET message = '%s', offset = %d, timestamp = %d where id = '%s'; UPDATE filedb.testinfo SET message = '%s', offset = %d, timestamp = %d where id = '%s';";//更新数据库，需同时更新内存、文件数据库中的内容
const char* sql_search_data = "SELECT * FROM MAIN.testinfo WHERE timestamp BETWEEN %d AND %d union SELECT * FROM testdb.testinfo WHERE timestamp BETWEEN %d AND %d;"; //查找数据库，将内存、文件数据库中查找出的内容合并
const char* sql_transfer_data = "INSERT OR REPLACE INTO filedb.testinfo SELECT * FROM testinfo;";   //将内存数据库中的信息同步到文件数据库中
const char* sql_delete_memory_table = "DELETE FROM testinfo;";	//内存数据库中的内容同步结束后，清空


int InsertRecord(DATA_TYPE type, const char* id, const char* message, int offset, int timestamp)
{
    int      rc              =  0;
    char*    errMsg          =  NULL;
    char     sqlcmd[512]     =  {0};
    time_t   insertTimestamp =  0;

    snprintf(sqlcmd, sizeof(sqlcmd), sql_insert_data, id, message, offset, timestamp);
    rc = sqlite3_exec(memdb, sqlcmd, NULL, NULL, &errMsg);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "cat't add record to memory database %s, sqlcmd=%s, err:%s\n", map_data_table[type].data_table_name, sqlcmd, errMsg);
        return -1;
    }
    
    return 0;
}

int UpdateRecord(DATA_TYPE type, const char* id, const char* message, int offset, int timestamp)
{
    int      rc              = 0;
    char*    errMsg          = NULL;
    char     sqlCmd[512]  = {0};

    snprintf(sqlCmd, sizeof(sqlCmd), sql_update_data, message, offset, timestamp, id, message, offset, timestamp, id);
    rc = sqlite3_exec(memdb, sqlCmd, NULL, NULL, &errMsg);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "cat't update record %s:%s\n", map_data_table[type].data_table_name, errMsg);
        return -1;
    }

    return 0;
}

int DeleteRecord(DATA_TYPE type, const char* id)
{
    int      rc              =  0;
    char*    errMsg          =  NULL;
    char     sqlcmd[512]     =  {0};

    snprintf(sqlcmd, sizeof(sqlcmd), sql_delete_data, id,  id);
    rc = sqlite3_exec(memdb, sqlcmd, NULL, NULL, &errMsg);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "cat't delete record %s:%s\n", map_data_table[type].data_table_name, errMsg);
        return -1;
    }

    return 0;
}

int QueryMessage(DATA_TYPE type, int startTime, int endTime)
{
    int      rc              = 0;
    char     *errMsg         = NULL;
    sqlite3  *filedb         = NULL;
    char**   pRecord         = NULL;
    int      row             = 0;
    int      column          = 0;
	char     sqlcmd[512]     = {0};

    if (type > VEP_NELEMS(map_data_table) || type < 0) {
        return -1;
    }

    rc = sqlite3_open(file_database_path, &filedb);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "cat't open database:%s\n", sqlite3_errmsg(filedb));
        sqlite3_close(filedb);
        return -1;
    }

    snprintf(sqlcmd, sizeof(sqlcmd), sql_search_data,  startTime, endTime,  startTime, endTime);
    
    rc = sqlite3_get_table(filedb, sqlcmd, &pRecord, &row, &column, &errMsg);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "cat't get table from%s:%s\n", map_data_table[type].data_table_name, errMsg);
        return -1;
    }
	
    int i;
    printf("row = %d, column = %d\n", row, column);
    for(i = 0; i < 2*column; i++)
    {
        printf("%s ", pRecord[i]);
    }
    printf("\n");

    return 0;
}

//定时调用此函数将内存数据中的内容同步到文件数据库
int Flush(){
    int      i            = 0;
    int      rc           = 0;
    char*    errMsg       = NULL;
    char     sqlcmd[512]  = {0};

	snprintf(sqlcmd, sizeof(sqlcmd), sql_transfer_data);
	rc = sqlite3_exec(memdb, sqlcmd, NULL, NULL, &errMsg);
	if (SQLITE_OK != rc) {
		fprintf(stderr, "cat't transfer memory database %s to file databasede:%s\n", map_data_table[i].data_table_name, sqlite3_errmsg(memdb));
		sqlite3_close(memdb);
		return -1;
	}
	snprintf(sqlcmd, sizeof(sqlcmd), sql_delete_memory_table);
	rc = sqlite3_exec(memdb, sqlcmd, NULL, NULL, &errMsg);

    return 0;
}

//创建文件数据库
int CreateDbOnFile()
{
    sqlite3 *db           = NULL;
    int      rc           = 0;
    char*    errMsg       = NULL;
    char     sqlcmd[512]  = {0};
    int      i            = 0;

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
    int      i            = 0;

    rc = sqlite3_open(":memory:", &memdb);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "cat't open database:%s\n", sqlite3_errmsg(memdb));
        sqlite3_close(memdb);
        return -1;
    }
    
	snprintf(sqlcmd, sizeof(sqlcmd), sql_create_data);
	rc = sqlite3_exec(memdb, sqlcmd, NULL, NULL, &errMsg);
	if (SQLITE_OK != rc) {
		fprintf(stderr, "cat't create memory database %s\n", errMsg);
		sqlite3_close(memdb);
		return -1;
	}

    return 0;
}

//解绑数据库
int DetachDb()
{
    int      rc           =  0;
    char*    errMsg       =  NULL;
    char     sqlcmd[512]  =  {0};

    snprintf(sqlcmd, sizeof(sqlcmd), "DETACH '%s'", "filedb");
    rc = sqlite3_exec(memdb, sqlcmd, NULL, NULL, &errMsg);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "detach file database failed:%s:%s\n", file_database_path, errMsg);
        sqlite3_close(memdb);
        return -1;
    }

    return 0;
}

//将文件数据库作为内存数据库的附加数据库
int AttachDb()
{
    int      rc           =  0;
    char*    errMsg       =  NULL;
    char     sqlcmd[512]  =  {0};

    snprintf(sqlcmd, sizeof(sqlcmd), "ATTACH '%s' AS %s", file_database_path, "filedb");
    rc = sqlite3_exec(memdb, sqlcmd, NULL, NULL, &errMsg);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "cat't attach database %s:%s\n", file_database_path, errMsg);
        sqlite3_close(memdb);
        return -1;
    }

    return 0;
}

//初始化数据库，分别创建文件数据库、内存数据库并把文件数据库attach到内存数据库上
int InitSqliteDb()
{
    int retval = 0;

    retval =  CreateDbOnFile();
    if (retval != 0) {
        return retval;
    }

    retval =  CreateDbOnMemery();
    if (retval != 0) {
        return retval;
    }

    retval =  AttachDb();
    if (retval != 0) {
        return retval;
    }

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
