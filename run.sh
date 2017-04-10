#!/bin/bash
rm a.out
g++ source/*.cpp source/*.c -I include -std=c++11 -lSDL2main -lSDL2
./a.out
