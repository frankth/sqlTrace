#ifndef SQLTRACE_H_
#define SQLTRACE_H_
/*
 * (C) 2016 Frank Thomas (postproc@inside-tec.de)
 * Licence: MIT license (see LICENSE.txt)
 */
#include <time.h>

typedef struct sqlTraceInfo_t sqlTraceInfo_t;
struct sqlTraceInfo_t {
    sqlTraceInfo_t *next;
    const char *fileName;
    int line;
    const char *stmt;
    int cnt;
    struct timespec usedTime;
} ;
void sqlTracePrintTopSql(void);
void sqlTraceResetStat(void);
void sqlTraceDumpRingbuffer(void);

#endif /* SQLTRACE_H_ */
