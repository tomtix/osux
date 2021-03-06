#!/bin/bash

if [[ $0 != ./`basename $0` ]]; then
    echo "DONT LAUNCH THIS SCRIPT OUTSIDE OF ITS DIRECTORY!"
    exit 1
fi

mkdir -p ../build/release
cd ../build/release
if [ ! -f Makefile ]; then
    rm -rf *
    cmake -DCMAKE_BUILD_TYPE=Release -H../.. -B.
fi
make -s install
