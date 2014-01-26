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

TESTS = $(wildcard *_test.cc)
SRCS = $(filter-out $(TESTS),$(wildcard *.cc))

all: Server Obsequi tests
#tests: objs/bitops_test
tests: $(TESTS:%.cc=objs/%)

# Domineering Solver, commandline controlled
O_OBJSX= obsequi.o base.o board.o negamax.o positional-values.o \
	move.o transposition.o stats.o bitops.o score-board.o
O_OBJS= $(addprefix objs/,$(O_OBJSX))
Obsequi: $(O_OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

# Domineering Solver, with HTML UI
S_OBJSX= main.o base.o protoson.o http.o server.o obsequi.pb.o \
	board.o positional-values.o transposition.o \
	score-board.o bitops.o
S_OBJS= $(addprefix objs/,$(S_OBJSX))
Server: $(S_OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ `pkg-config --libs protobuf`

# Tests
objs/bitops_test: bitops_test.cc objs/bitops.o
objs/board_test: board_test.cc objs/move.o objs/board.o objs/bitops.o \
    objs/base.o objs/transposition.o objs/positional-values.o
objs/move_test: move_test.cc objs/move.o objs/board.o objs/bitops.o \
    objs/base.o objs/transposition.o objs/positional-values.o

# List of implicit rules
%.pb.cc %.pb.h : %.proto
	protoc --cpp_out=. $<

objs/%.pb.o : %.pb.cc
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# No idea why pb.h files needs to be listed twice, but otherwise it deletes it.
objs/%.o : %.cc
	$(CC) $(CPPFLAGS) -MM $< -MG -MT $@ > objs/$*.D
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

objs/%_x.o : %.cc
	$(CC) $(CPPFLAGS) -MM $< -MG -MT $@ > objs/$*_x.D
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

objs/%_test : %_test.cc
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
	valgrind --error-exitcode=1 --leak-check=full ./$@

## other stuff
.PHONY: clean

clean:
	/bin/rm -f obsequi.pb.* objs/*

-include $(SRCS:%.cc=objs/%.D)
-include $(SRCS:%.cc=objs/%_x.D)
