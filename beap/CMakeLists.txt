add_library(beap beap.c)

option(HEAP_DEBUG "Enables more debugging info" OFF)

if (HEAP_DEBUG)
  target_compile_definitions(beap PRIVATE "HEAP_DEBUG")
endif()
