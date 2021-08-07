# masters_thesis

TODO:
- introduce concepts.
- add MPMC regression testing.
- lock free dynamic sized map.

Some tradeoffs to consider for const-sized lock free impl:
- how to handle erase queue. can you do it inplace? for now, we are having it as a seperate array.
- consider trade-off: when erasing, do you want to increment generator instantly, so that the element can no longer be randomly-accessed (requiring the generation to be atomic), or defer to the actual erase queue processing (making it possible to access the erased element until then). Currently implemented the latter.