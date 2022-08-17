#!/bin/bash

if [ $# != 2 ] || [[ ! $1 =~ ^("install"|"build")$ ]] || [[ ! $2 =~ ^("Release"|"Debug")$ ]] 
then
    echo "Usage ./make.sh [install|build] [Debug|Release]"
    exit
fi


if [ $1 = "install" ]
then
    # get submodules
    if [ !"$(ls SG14)" ]
    then
        pushd SG14 > /dev/null
        git submodule update --init --recursive
        popd > /dev/null
    fi

    # create build directory
    mkdir -p build
    mkdir -p build/$2

    pushd build/$2 > /dev/null
    cmake -DCMAKE_CXX_COMPILER=g++-11 -DCMAKE_BUILD_TYPE=$2 ../..
    popd > /dev/null
else
    pushd build/$2 > /dev/null
    cmake --build . -j 4
    popd > /dev/null
fi
