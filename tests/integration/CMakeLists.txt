# Integration tests
if(Catch2_FOUND)
    add_executable(IntegrationTests
        test_placeholder.cpp
        # test_opcua_integration.cpp
        # test_physics_integration.cpp
    )

    target_link_libraries(IntegrationTests PRIVATE
        Core
        Rendering
        Catch2::Catch2WithMain
    )

    target_include_directories(IntegrationTests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    include(CTest)
    include(Catch)
    catch_discover_tests(IntegrationTests)
endif()
