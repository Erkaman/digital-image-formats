#!/bin/bash

# if no command line paramters were supplied, run it normally.
if [ -z "$1" ]; then
    for file in  img/*.TGA; do ./tga "$file" "$file".dmp ; done
else
    for file in  img/*.TGA; do valgrind ./tga "$file" "$file".dmp ; done
fi
