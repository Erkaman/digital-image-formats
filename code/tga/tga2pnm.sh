#!/usr/bin/env bash

if [[ $# != 1 ]]
then
    echo "You must supply 1 argument."
    exit 1
fi

tgatoppm img/"$1" | pnmtoplainpnm > "$1".txt
