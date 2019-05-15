#!/bin/bash

LD_LIBRARY_PATH=../../import/lib; export LD_LIBRARY_PATH
sudo chmod 666 /dev/ttyACM0

./Sample 0 921600 STB_ON
