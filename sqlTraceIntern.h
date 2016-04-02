#ifndef SQLTRACEINTERN_H_
#define SQLTRACEINTERN_H_
/*
 * (C) 2016 Frank Thomas (postproc@inside-tec.de)
 * Licence: MIT license (see LICENSE.txt)
 */

#include "sqlTrace.h"

extern void sqlTraceSqlCxt(void *p1, void *p2, struct sqlexd *p3, const struct sqlcxp *fn, const char *stmt,sqlTraceInfo_t *ti);
#endif /* SQLTRACEINTERN_H_ */
