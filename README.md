# beap

A basic linked list-based heap made initially for [purpleK2](https://github.com/purpleK2/rtos).
Was originally meant as a drop-in replacement for liballoc, due to its dubious stability on 64-bit systems.

The library is available at the [`beap`](beap) directory, with a `CMakeLists.txt` for integrating it with your CMake project.

To implement it in your project, you can check the functions in [`beap/beap.h`](beap/beap.h).
You can add `HEAP_DEBUG` to your compiler definitions to enable some logging information.
(it's added by default on the library's CMakeLists)

For an example implementation, you can check out [`src/beap_wrappers.c`](src/beap_wrappers.c).

# Allocator benchmark

The provided [`src/test.c`](src/test.c) contains a simple test to allocate X (being provided as an argument, by default it's 1000) times, giving back the time taken for allocation (and deallocation).

To compile, run the following:

`cmake -B build .` to generate the build files
(note: you can add `-D HEAP_DEBUG=[ON/OFF]` to enable/disable logging.)

`make -C build` to compile the project and produce `beap_test` (in the root of the project)

run `./beap_test <number of allocations>` and wait for the results :)

Made by Omar (RepubblicaTech)

Licensed under MIT.
