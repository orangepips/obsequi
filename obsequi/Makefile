
##  blah
CC=gcc
CC=/usr/local/gcc/bin/gcc

CFLAGS= 
CPPFLAGS= `./cppflags`

# Use following options to improve performance by a couple percent.
#   first to profile.
#   second to recompile with the bonus of the profiling info.
# -fprofile-arcs
# -fbranch-probabilities

##  target files.
OBJS= obsequi.o utils.o \
      negamax.o position_values.o \
      toggle_move.o move_gen.o move_sort.o hash.o \
      display.o traits.o \
      tables.o init.o \
      does_x_win.o


EXE= Obsequi

##  target
$(EXE): cppflags $(OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(OBJS) -o $@


##  phony targets
.PHONY: hashclean tidy clean depend

hashclean:
	/bin/rm -f hash_big.o hash_big2.o hash_n1.o hash_n2.o hash.o

tidy:
	/bin/rm -f $(OBJS) gmon.out

clean:
	/bin/rm -f $(OBJS) $(EXE) *.da *.o gmon.out

depend:
	$(CC) $(CPPFLAGS) -MM $(patsubst %.o,%.c,$(OBJS)) > .depend~
	sed s/:/": cppflags"/ < .depend~ > .depend

## other stuff
cppflags.h: cppflags.raw
	./cppflags.gen cppflags.h

cppflags: cppflags.raw
	./cppflags.gen cppflags

## include the .depend file
include .depend
