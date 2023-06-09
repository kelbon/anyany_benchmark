# anyany_benchmark
This repository is a small comparison between type erasure library https://github.com/kelbon/AnyAny and C++ in-language virtual functions in commonly used patterns such as:
creating, invoking methods on erased objects etc.

Results on my machine:


![image](https://user-images.githubusercontent.com/58717435/227379776-eeda7144-dd83-4edc-85a0-1dbfdbd2163e.png)

![image](https://user-images.githubusercontent.com/58717435/227379835-37e75d9a-b1e6-4ea8-86da-7bc9e9bb997f.png)

Conclusions:

1. invoking:
  AnyAny and virtual functions have +- same performance for small arrays, because asm code +- equal, but for huge amount of objects anyany has a huge win,
  because of cash locality: in case of virtual functions processor must load value pointer and then start load pointer to vtable(and then - to function), while
  is AnyAny case processor can load vtable pointer and value pointer at same time(vtable pointer stored outside of object), moreover in many cases value pointer
  dont required, because value stored on stack(due SOO optimization) and vtable pointer already in processor's cache, because we have only one vtable for one type,
  but one value pointer for every value
2. copy: AnyAny is faster to create and copy, because it allocates less times and because of cache localicy things (see 1.)
3. sort: unique_ptr<Base> move operations(sort) may outperform AnyAny move/move assign, just because it is one std::exchange,
while AnyAny has self referencing structure, which require +- 3 std::exchange to move
