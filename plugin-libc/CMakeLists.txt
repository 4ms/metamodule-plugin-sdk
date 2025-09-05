add_library(metamodule-plugin-libc STATIC)
add_library(metamodule::plugin-libc ALIAS metamodule-plugin-libc)

if (METAMODULE_PLUGIN_STATIC_LIBC)
    target_sources(metamodule-plugin-libc PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/dso_handle.c
    )
else()
    include(libm.cmake)
    include(libc.cmake)
endif()

include(libsupcpp.cmake)
include(libstdcpp.cmake)


target_compile_options(metamodule-plugin-libc PRIVATE
	-fbuiltin
	-fno-math-errno

    # Do not export plugin-libc symbols (removes unused symbols and does not make GOT/PLT tables for them):
    -fvisibility=hidden 
)

