# obsequi 

latin for compliance, indulgence, submission, pliancy.

This program is able to play and solve domineering boards.
Currently it can solve 9x9 and larger boards with a single processor.

## Compilation

To compile cd to the directory "obsequi", type make.

Should work, if not, you must be using a different compiler or
libraries. (I use gcc and glibc).

## Usage

There isn't really any documentation for obsequi (sorry).

To solve an 8x8 board with a transposition table with (1 << 24) entries use:
echo "solve rows 8 cols 8 bits 24 V" | ./Obsequi

You can also pre-place blocks on the board.
echo "solve rows 8 cols 8 bits 24 b0,1;1,1 V" | ./Obsequi

The V says it is verticals turn first.

## OSX Installation Notes

Run `make` twice (see: https://github.com/nathanbullock/obsequi/issues/1)

```
$ make
g++ -std=c++11 -Wall -Werror -Wshadow -O3 -march=native -g  -MM main.cc -MG -MT objs/main.o > objs/main.D
g++ -std=c++11 -Wall -Werror -Wshadow -O3 -march=native -g   -c main.cc -o objs/main.o
main.cc:6:10: fatal error: 'obsequi.pb.h' file not found
#include "obsequi.pb.h"
         ^
1 error generated.
make: *** [objs/main.o] Error 1
```

Create the directory `obsequi/objs` to solve the error 

```
$make
g++ -std=c++11 -Wall -Werror -Wshadow -O3 -march=native -g -rdynamic -MM main.cc -MG -MT objs/main.o > objs/main.D
/bin/sh: objs/main.D: No such file or directory
make: *** [objs/main.o] Error 1
```

In transposition.cc add `#include <stdlib.h>` to address 

```
transposition.cc:18:3: error: use of undeclared identifier 'srandom'
  srandom(1);
  ^
transposition.cc:20:18: error: use of undeclared identifier 'random'
    zobrist[i] = random();
                 ^
```

Comment out `-rdynamic` and `CPPFLAGS+= -Wall -Werror -Wshadow` in Makefile

## Dependencies

Install [Protocol Buffers](http://brewformulas.org/Protobuf)

`$ brew update`
`$ brew install protobuf` 

Valgrind 

`$ brew install Valgrind`