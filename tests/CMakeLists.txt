# Tests configuration
find_package(Catch2 3 QUIET)

if(Catch2_FOUND)
    # Unit tests
    add_subdirectory(unit)

    # Integration tests
    add_subdirectory(integration)

    message(STATUS "Tests will be built with Catch2")
else()
    message(WARNING "Catch2 not found, tests will not be built")
endif()
