#!/bin/bash
shopt -s nocaseglob
for file in  img/*.tga; do ./tga "$file" ; done