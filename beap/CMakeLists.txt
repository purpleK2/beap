add_library(beap beap.c)

option(HEAP_DEBUG "Enables more debugging info" OFF)

set(CMAKE_MESSAGE_LOG_LEVEL VERBOSE)

if (BEAP_DEBUG)
  message(VERBOSE "Beap logging is enabled")
  target_compile_definitions(beap PRIVATE BEAP_DEBUG)
endif(BEAP_DEBUG)

if (NOT BEAP_PAGE)
	message(FATAL_ERROR "BEAP_PAGE is not defined. Define and assign to it the size of your page frames.")
	return(-1)
endif(NOT BEAP_PAGE)
message(VERBOSE "BEAP_PAGE is ${BEAP_PAGE}")
target_compile_definitions(beap PUBLIC BEAP_PAGE=${BEAP_PAGE})
