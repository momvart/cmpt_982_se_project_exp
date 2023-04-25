#!/bin/bash

set -e

name=`echo "${1%.*}"`
rustc --emit=llvm-ir $1 -o $name.ll
./svf-ex $name.ll
