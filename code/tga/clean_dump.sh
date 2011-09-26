#!/bin/bash
shopt -s nocaseglob
for file in img/*.dmp; do rm "$file" ; done