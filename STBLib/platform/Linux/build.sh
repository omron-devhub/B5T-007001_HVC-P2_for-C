#!/bin/bash

(gcc -v 2>&1) > make.log

make -f Makefile allclean
make -f Makefile all 2>&1 | tee >> make.log

