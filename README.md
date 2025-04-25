# beap

A TLSF heap made initially for [purpleK2](https://github.com/purpleK2/rtos).
Was originally meant as a drop-in replacement for liballoc, due to its dubious stability on 64-bit systems.

The library is available at the [`beap`](beap) directory, with a `CMakeLists.txt` for integrating it with your CMake project.

All kernel API are available at [`beap/beap.h`](beap/beap.h).

For an example implementation, you can check out [`src/beap_wrappers.c`](src/beap_wrappers.c).

# Allocator benchmark

The provided [`src/test.c`](src/test.c) contains a simple test to allocate X (being provided as an argument, by default it's 1000) times, giving back the time taken for allocation (and deallocation).

Run `make` to compile the project and produce `beap_test` (in the root of the project)

run `./beap_test <number of allocations>` and wait for the results :)

Credits go to [mattconte's TLSF](https://github.com/mattconte/tlsf).

Licensed under MIT.
