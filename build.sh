#!/bin/bash

if [ ! -d build/ ]; then
    mkdir build
fi

cd build
cmake ..
if [ $? -ne 0]; then
    echo "failed to configure cmake project"
    exit 1
fi

make
if [ $? -ne 0]; then
    echo "failed to build project"
    exit 1
fi

exit 0
