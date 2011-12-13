#!/usr/bin/env bash

if [[ $# < 1 ]]
then
    echo "Please supply at least 1 argument"
    exit 1
fi

shopt -s nocaseglob

val=false
if [[ "$1" = "-v" ]]
then
    val=true
    shift
fi

for file in  img/*."$1"
do
    echo "Dumping $file ..."
    if $val
    then
	valgrind ./"$1" "$file" "$file".dmp
    else
	./"$1" "$file" "$file".dmp
    fi
done
