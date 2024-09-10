#!/bin/sh
rm $1
eval cc $1.c $(pkg-config --libs --cflags raylib) -o $1
./$1