/*
*******************************************************************************
*                    Copyright (c) 2013, TQ System
*                             All Rights Reserved
*
* Purpose           : file
* File Name         : .c

* Revision history  :
    01a,2013/07/29,Ryan  -- Creation(written)

* Description       :

*******************************************************************************
*/

#ifndef _SQLITE3_TASK_INCLUDED_
#define _SQLITE3_TASK_INCLUDED_

#define RTDATA     0X01 //real time sample data


void write_sqliteFifo(unsigned char *wbuf, unsigned int len,unsigned char type);
void read_sqliteFifo(unsigned char *rbuf);

#endif

