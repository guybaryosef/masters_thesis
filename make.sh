#!/bin/bash

# get submodules
if [ !"$(ls SG14)" ]
then
    pushd SG14 > /dev/null
    git submodule update --init --recursive
    popd > /dev/null
fi

if [ $# -eq 0 ]
then
    echo "Need to specify Debug\Release build type"
    exit
fi

# create build directory
mkdir -p build
mkdir -p build/$1

pushd build/$1 > /dev/null

cmake -DCMAKE_CXX_COMPILER=g++-11 -DCMAKE_BUILD_TYPE=$1 ../..
cmake --build . -j 4

popd > /dev/null
