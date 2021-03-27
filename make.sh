#!/bin/bash

[ ! -d build ] && mkdir build

pushd build

cmake ..
cmake --build .

popd
