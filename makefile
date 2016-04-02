# (C) 2016 Frank Thomas (postproc@inside-tec.de)
# Licence: MIT license (see LICENSE.txt)
#
all: sample

CC=gcc
CFLAGS=-I$(ORACLE_HOME)/precomp/public -g
OBJS = sample.o sqlTrace.o
EXE=sample procIntern.h
LD_FLAGS=-L$(ORACLE_HOME)/lib -lclntsh -g
PROC_FLAGS = code=ansi_c

clean:
	rm -f $(EXE) $(OBJS)

procIntern.h: extrProcIntern
	echo >dummy.pc
	proc iname=dummy.pc $(PROC_FLAGS)
	extrProcIntern dummy.c > $@
	rm dummy.pc dummy.lis dummy.c

%.o: %.pc
	proc iname=$< oname=$(<:.pc=.gen.p) $(PROC_FLAGS)
	@rm $(<:.pc=.lis)
	postProc $(<:.pc=.gen.p) $(<:.pc=.gen.c)
	@rm $(<:.pc=.gen.p)
	$(CC) -c -o $@ $(<:.pc=.gen.c) $(CFLAGS)
	@rm $(<:.pc=.gen.c)

sqlTrace.o: sqlTrace.c procIntern.h

sample: sample.o sqlTrace.o
	gcc -o $@ $(LD_FLAGS) $^

