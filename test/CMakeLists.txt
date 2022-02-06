
# the different slot maps
add_subdirectory(LockedSlotMapUnitTests)
add_subdirectory(LockFreeSlotMapUnitTests)
add_subdirectory(LockFreeConstSizedSlotMapUnitTests)

# internal helper data structures
add_subdirectory(LockFreeVectorUnitTests)
add_subdirectory(InternalVectorUnitTests)

# regression tests
add_subdirectory(SlotMapRegressionTests)

# benchmark tests
