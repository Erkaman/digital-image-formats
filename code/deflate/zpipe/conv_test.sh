#!/bin/bash
BASE="$( cd -P "$( dirname "$0" )" && pwd )"
cmd="${BASE}/a ${BASE}/test ${BASE}/test.zlib"
$cmd
cp "${BASE}/test.zlib" "/cygdrive/c/Users/Eric/Dropbox/school_project/code/deflate/test.zlib"

