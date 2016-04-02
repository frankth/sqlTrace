# sqlTrace

## Automatic Oracle performance monitoring for Pro\*C programs

### Introduction
This is a postprocessor for Oracle Pro\*C files
See: <http://blog.inside-tec.de/2016/03/automatic-oracle-performance-monitoring.html>

The idea is to automatically gather information (e.g. performance data). This version works on Linux but is easily adaptable to other environments. Probably the most complex change would be regarding the high-res timer. Think of this as a proof of concept. E.g. The function `autoExitHandler` is automatically called on exit and prints the top SQL's taking more than 1% of the time and the last 20 SQLs and their `SQLCODE` to stdout. This may not make much sense in most applications, as they have individual logging mechanisms, or may not want to log at program exit, or ...
You may also want to extend the approach to capture other calls in similar ways. Normally you could initialize the static sqlTraceInfo\_t structure using macros to fill in `__FILE__` and `__LINE__` directly at compile time (if your compiler properly merges all references of `__FILE__` into one). With Pro\*C it is a bit more complex as you want to get at the line numbers of the original code.
Don't be to scared by this internal hacking of Pro\*C. Although Oracle could obviously change the inner structure of the generated code, this method works for more than 10 years (probably much longer).

###Ways this can be enhanced:

  - make it thread save. The places that need those enhancements are marked. That is only necessary if use `EXEC SQL` in different threads.
  - Basically makefiles are a mess and scons ( http://www.scons.org/ ) should be used.
  - if you use Pro\*C with code=kr\_c (the default) or cpp you may have to tweak extrProcIntern and postProc

###Usage
Take a look at the makefile. Basically the idea is simple.

 - generate procIntern.h by compiling an empty \*.pc file and extracting
   the relevant structures as they don't exist in a header file.
 - run postProc after proc and before compiling
 - link with sqlTrace.o added

###Sample output

    [oracle@vbgeneric sqlTrace]$ ./sample scott/tiger
    
    Connected to ORACLE
    Enter enter number (0 to quit): 1
    res=1
    
    Enter enter number (0 to quit): 2
    SQLCODE=1403
    
    Enter enter number (0 to quit): 1x
    ORA-01722: invalid number
    
    0.131898998 1 sample.pc:28 
    0.001452876 1 sample.pc:17 
    SQLCODE:00000 sample.pc:28 
    SQLCODE:00000 sample.pc:42 select 1 into :b0  from dual where 1=:b1
    SQLCODE:01403 sample.pc:42 select 1 into :b0  from dual where 1=:b1
    SQLCODE:-1722 sample.pc:42 select 1 into :b0  from dual where 1=:b1
    SQLCODE:00000 sample.pc:17 

###Public interface:
```C
void sqlTracePrintTopSql(void); // print top 20 SQLs based on elapsed time to stdout
void sqlTraceResetStat(void); // Start the statistics from scratch
void sqlTraceDumpRingbuffer(void); // Dump the contents of the ringbuffer to stdout
```

(C) 2016 Frank Thomas (postproc@inside-tec.de)
Licence: MIT license (see LICENSE.txt)

Nevertheless I would be happy if you send me an e-mail to postproc@inside-tec.de if you use this code in your projects.
If you did changes that would be useful for others I would be happy to hear from you as well.



