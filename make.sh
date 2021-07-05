#!/bin/bash

# get submodules
if [ !"$(ls SG14)" ]
then
    pushd SG14
    git submodule update --init --recursive
    popd
fi


# create build directory
[ ! -d build ] && mkdir build || rm build/bin/* build/libGBY_SlotMap.a


pushd build

cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# run unit tests
./bin/SlotMapUnitTests

popd
