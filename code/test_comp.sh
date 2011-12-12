#!/bin/sh

if [ -n "$1" ]; then
./"$1" -p ../testfiles/alice29.txt alice.cmp
./"$1" -d -p alice.cmp alice.unc
fi
