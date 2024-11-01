#!/bin/sh

set xe

mkdir -p bin

gcc -o bin/main main.c -lpng -O3 -Wall -Wextra
