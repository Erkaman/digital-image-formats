#!/bin/bash
for file in  img/*.TGA; do ./tga "$file" "$file".dmp ; done