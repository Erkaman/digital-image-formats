#!/bin/sh

if [ -n "$1" ]; then
valgrind ./"$1" -p ../testfiles/alice29.txt alice.cmp
valgrind ./"$1" -d -p alice.cmp alice.unc
fi
