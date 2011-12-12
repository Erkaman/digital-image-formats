#!/usr/bin/env bash

if [[ $# < 1 ]]
then
    echo "Please supply at least 1 argument"
    exit 1
fi

if [[ "$1" = "-v" ]]
then
    shift
    CMD="valgrind ./$1"
else
    CMD="./$1"
fi

if [[ -n "$2" ]]
then
    CMD="$CMD $2"
fi

eval $CMD ../testfiles/alice29.txt alice.cmp
eval $CMD -d alice.cmp alice.unc
