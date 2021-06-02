
target_sources(SlotMap PRIVATE
    locked_slot_map.h
    main.cpp)

add_subdirectory(lock_free_impl)
