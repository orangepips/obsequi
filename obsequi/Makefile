CC=g++
CPPFLAGS= -std=c++11

# Lots of warnings and make them all errors.
CPPFLAGS+= -Wall -Werror -Wshadow

# Turn on profiling.
# Run Obsequi
# gprof Obsequi gmon.out | less
# CPPFLAGS += -pg

# Only the best optimization for this machine.
CPPFLAGS += -O3 -march=native

# Create better debug symbols.
CPPFLAGS += -g -rdynamic


# TODO: pb.cc files should go in objs/
TESTS = $(wildcard *_test.cc)
SRCS = $(filter-out $(TESTS),$(wildcard *.cc))

all: Obsequi tests
#tests: objs/bitops_test
tests: $(TESTS:%.cc=objs/%)

# Domineering Solver, commandline controlled
O_OBJSX= obsequi.o utils.o board.o negamax.o positional-values.o \
	move.o hash-table.o stats.o lastbit.o countmoves.o score-board.o
O_OBJS= $(addprefix objs/,$(O_OBJSX))
Obsequi: $(O_OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

# Tests
objs/bitops_test: bitops_test.cc objs/bitops.o
objs/move_test: move_test.cc objs/move.o objs/board.o objs/bitops.o \
    objs/base.o objs/hash-table.o objs/positional-values.o

objs/%.o : %.cc
	$(CC) $(CPPFLAGS) -MM $< -MT $@ > objs/$*.D
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

objs/%_x.o : %.cc
	$(CC) $(CPPFLAGS) -MM $< -MT $@ > objs/$*_x.D
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

objs/%_test : %_test.cc
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
	valgrind --leak-check=full ./$@

## other stuff
.PHONY: clean

clean:
	/bin/rm -f obsequi.pb.* objs/*

-include $(SRCS:%.cc=objs/%.D)
-include $(SRCS:%.cc=objs/%_x.D)
