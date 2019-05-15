#!/bin/bash

make -f Makefile clean
make -f Makefile all 2>&1 | tee Log.log

