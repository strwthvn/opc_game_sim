# Industrial module - промышленные протоколы
add_library(Industrial STATIC)

target_sources(Industrial
    PRIVATE
        # Здесь будут добавлены исходные файлы
        # OPCUAClient.cpp
        # PLCBindingSystem.cpp
        # IndustrialComponents.cpp
)

target_include_directories(Industrial
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include/industrial
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/industrial
)

target_link_libraries(Industrial
    PUBLIC
        Core
        open62541::open62541
        Boost::signals2
        EnTT::EnTT
    PRIVATE
)

# Опциональная поддержка Modbus
if(ENABLE_MODBUS)
    target_sources(Industrial PRIVATE
        # ModbusClient.cpp
    )
    target_link_libraries(Industrial PUBLIC libmodbus::libmodbus)
endif()

target_compile_features(Industrial PUBLIC cxx_std_20)
