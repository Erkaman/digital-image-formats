#!/usr/bin/env bash

shopt -s nocaseglob

for file in img/*."$1".dmp
do
    echo "Removing $file"
    rm "$file"
done
