#!/bin/bash

[ ! -d build ] && mkdir build || rm build/bin/* build/libGBY_SlotMap.a

pushd build


cmake ..
cmake --build .

./bin/LockedSlotMapUnitTest

popd
