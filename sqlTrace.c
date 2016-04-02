/*
 * (C) 2016 Frank Thomas (postproc@inside-tec.de)
 * Licence: MIT license (see LICENSE.txt)
 */
#include "procIntern.h"
#include "sqlTraceIntern.h"
#include <stdio.h>
#include <stdlib.h>
#include <sqlca.h>

// ringbuffer for the last n sql statements. Might be usefull in case of problems, to see what led to it.
#define RINGBUF_SIZE 20
static struct ringBuffer_t {
    sqlTraceInfo_t *ti;
    int sqlcode;
} ringBuffer[RINGBUF_SIZE];
static int ringBufferIdx=0;
static sqlTraceInfo_t tiLast;
static sqlTraceInfo_t *tiHeader=&tiLast; //initializing it to null directly can lead to endless loops if the first statement (e.g. after a resetStat) is executed twice in a row.

#define NANOSECS_PER_SEC 1000000000

void sqlTraceSqlCxt(void *p1, void *p2, struct sqlexd *exd, const struct sqlcxp *fn, const char *stmt,sqlTraceInfo_t *ti) {
    if (!ti->next) {
	const short *curSqlCud=exd->cud+exd->offset;
	/* todo; thread save */
	ti->next = tiHeader;
	tiHeader = ti;
	/* end thread save */
	ti->fileName=fn->filnam;
	ti->line=curSqlCud[8]*8192+curSqlCud[7];
	ti->stmt=stmt;
    }
    struct timespec start,end;
    clock_gettime(CLOCK_MONOTONIC_RAW,&start);
    sqlcxt(p1,p2,exd,fn);
    clock_gettime(CLOCK_MONOTONIC_RAW,&end);
    /* todo; thread save */
    ringBuffer[ringBufferIdx].ti=ti;
    ringBuffer[ringBufferIdx].sqlcode=((struct sqlca *)exd->sqlest)->sqlcode;
    ringBufferIdx=(ringBufferIdx+1)%RINGBUF_SIZE;
    ti->cnt++;
    ti->usedTime.tv_sec+=(end.tv_sec-start.tv_sec);
    ti->usedTime.tv_nsec+=(end.tv_nsec-start.tv_nsec);
    if (ti->usedTime.tv_nsec<0) {
	ti->usedTime.tv_sec--;
	ti->usedTime.tv_nsec+=NANOSECS_PER_SEC;
    } else if (ti->usedTime.tv_nsec >= NANOSECS_PER_SEC) {
	ti->usedTime.tv_sec++;
	ti->usedTime.tv_nsec-=NANOSECS_PER_SEC;
    }
    /* end thread save */
}

static void addTimeSpec(struct timespec *dst,const struct timespec *src)
{
    dst->tv_sec+=src->tv_sec;
    dst->tv_nsec+=src->tv_nsec;
    if (dst->tv_nsec >= NANOSECS_PER_SEC) {
	dst->tv_sec++;
	dst->tv_nsec-=NANOSECS_PER_SEC;
    }
}

static int cmpTimeSpec(struct timespec *a,struct timespec *b)
{
    int res=a->tv_sec-b->tv_sec;
    if (res==0) res=a->tv_nsec-b->tv_nsec;
    return res;
}

static int cmpTraceInfoDesc(sqlTraceInfo_t **a, sqlTraceInfo_t **b)
{
    return cmpTimeSpec(&(*b)->usedTime,&(*a)->usedTime);
}

// print sqls taking more than 1% of the time
// Show everything larger than 1/MIN_PARTS part of the elapsed time
#define MIN_PARTS 100
void sqlTracePrintTopSql(void)
{
    sqlTraceInfo_t *topSql[MIN_PARTS]; // can not get more than that!
    struct timespec sum={0,0};

    sqlTraceInfo_t *ti;
    for(ti=tiHeader;ti;ti=ti->next) {
	addTimeSpec(&sum,&ti->usedTime);
    }
    struct timespec minTime;
    minTime.tv_sec=sum.tv_sec/MIN_PARTS;
    minTime.tv_nsec=(sum.tv_sec%MIN_PARTS)*(NANOSECS_PER_SEC/MIN_PARTS)+sum.tv_nsec/MIN_PARTS;

    sqlTraceInfo_t **freeEntry=topSql;
    for(ti=tiHeader;ti;ti=ti->next) {
	if(cmpTimeSpec(&ti->usedTime,&minTime)>0) *(freeEntry++)=ti;
    }
    qsort(topSql,freeEntry-topSql,sizeof(*freeEntry),(int (*)(const void *, const void *))cmpTraceInfoDesc);
    sqlTraceInfo_t **e;
    for(e=topSql;e<freeEntry;e++) {
	sqlTraceInfo_t *ti=*e;
	printf("%ld.%09ld %d %s:%d %s\n",ti->usedTime.tv_sec,ti->usedTime.tv_nsec,ti->cnt,ti->fileName,ti->line,ti->stmt?ti->stmt:"");
    }
}

void sqlTraceResetStat(void)
{
    sqlTraceInfo_t **kill=&tiHeader->next;
    sqlTraceInfo_t *ti;
    while(ti=*kill) {
	ti->cnt=0;
	ti->usedTime.tv_sec=0;
	ti->usedTime.tv_nsec=0;
	*kill=NULL;
	kill=&ti->next;
    }
    tiHeader=&tiLast;
}

void sqlTraceDumpRingbuffer(void)
{
    int printIdx=ringBufferIdx;
    do {
	struct ringBuffer_t *rb=&ringBuffer[printIdx];
	sqlTraceInfo_t *ti = rb->ti;
	if (ti) {
	    printf("SQLCODE:%05d %s:%d %s\n",rb->sqlcode,ti->fileName,ti->line,ti->stmt?ti->stmt:"");
	}
	printIdx=(printIdx+1)%RINGBUF_SIZE;
    } while(printIdx!=ringBufferIdx);
}

static void __attribute__((destructor)) autoExitHandler(void)
{
    sqlTracePrintTopSql();
    sqlTraceDumpRingbuffer();
}

