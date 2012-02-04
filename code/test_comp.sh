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

eval $CMD ../testfiles/kennedy.xls  kennedy.cmp
eval $CMD -d kennedy.cmp kennedy.unc

if diff "../testfiles/kennedy.xls" "kennedy.unc" >/dev/null ; then
  echo "Resulting files equal!"
else
  echo "Resulting files not equal!!!"
fi