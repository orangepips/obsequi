CC=g++
CFLAGS= 
CPPFLAGS= `./cppflags`

EXE= Obsequi
OBJS= obsequi.o utils.o board.o \
      negamax.o positional-values.o \
      move.o hash-table.o display.o \
      init.o lastbit.o countmoves.o \
      score-board.o

$(EXE): $(OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(OBJS) -o $@

%.o : %.cc cppflags.h
	$(CC) $(CPPFLAGS) -MM $< > $*.D
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

## other stuff
cppflags.h: cppflags
	./cppflags header > cppflags.h

.PHONY: clean

clean:
	/bin/rm -f $(EXE) *.da *.o *.D gmon.out cppflags.h err core

-include $(OBJS:.o=.D)
