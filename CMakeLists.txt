project(Metamodule-SDK)

# rack-interface: library to interface with RackSDK adaptor
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/rack-interface rack-interface)

# core-interface: library to interface with MetaModule CoreModule
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/core-interface core-interface)

# plugin-libc: provides libc and libstdc++ for plugins
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/plugin-libc plugin-libc)

# cpputil
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/cpputil cpputil)

# Create Plugin SDK library
add_library(metamodule-sdk INTERFACE)

target_link_libraries(metamodule-sdk INTERFACE 
    metamodule::rack-interface
    metamodule::core-interface
    metamodule::plugin-libc
    cpputil
)
