project(Metamodule-SDK)

# metamodule-rack-interface: library to interface with RackSDK adaptor
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/metamodule-rack-interface metamodule-rack-interface)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/metamodule-core-interface metamodule-core-interface)
# add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/metamodule-plugin-libc metamodule-plugin-libc)

# cpputil
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/cpputil cpputil)

# Plugin library
add_library(metamodule-sdk STATIC)

target_sources(metamodule-sdk PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/libc_stub.c
)

target_link_libraries(metamodule-sdk PUBLIC 
    metamodule-rack-interface
    metamodule-core-interface
    # metamodule-plugin-libc
    cpputil
)
