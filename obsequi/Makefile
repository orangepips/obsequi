CC=g++
CFLAGS= 
CPPFLAGS= `./cppflags`

EXE= Obsequi
OBJS= obsequi.o utils.o \
      negamax.o position_values.o \
      toggle_move.o move-gen.o move_sort.o hash.o \
      display.o \
      tables.o init.o lastbit.o countmoves.o \
      does_x_win.o

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
	/bin/rm -f $(EXE) *.da *.o *.D gmon.out cppflags.h

-include $(OBJS:.o=.D)
