# Unit tests
if(Catch2_FOUND)
    add_executable(UnitTests
        # test_main.cpp
        # test_core.cpp
        # test_simulation.cpp
    )

    target_link_libraries(UnitTests PRIVATE
        Core
        Simulation
        Rendering
        Industrial
        Scripting
        Editor
        UI
        Catch2::Catch2WithMain
    )

    include(CTest)
    include(Catch)
    catch_discover_tests(UnitTests)
endif()
