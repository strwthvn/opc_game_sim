# Integration tests
if(Catch2_FOUND)
    add_executable(IntegrationTests
        # test_opcua_integration.cpp
        # test_physics_integration.cpp
    )

    target_link_libraries(IntegrationTests PRIVATE
        Core
        Simulation
        Industrial
        Catch2::Catch2WithMain
    )

    include(CTest)
    include(Catch)
    catch_discover_tests(IntegrationTests)
endif()
