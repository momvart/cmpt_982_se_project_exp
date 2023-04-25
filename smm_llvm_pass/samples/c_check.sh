#!/bin/bash

set -e

name=`echo "${1%.*}"`
clang -S -c -g -fno-discard-value-names -emit-llvm $1 -o $name.ll
./svf-ex $name.ll
