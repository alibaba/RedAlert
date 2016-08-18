#!/bin/sh
PY_INC=/usr/include/python2.4
gcc -pthread -fno-strict-aliasing -DNDEBUG -g -fwrapv -O3 -Wall -Wstrict-prototypes -fPIC -I${PY_INC} -c _speedups.c -o _speedups.o
gcc -pthread -shared _speedups.o -o _speedups.so