cmake_minimum_required(VERSION 3.22)

project(core-interface-tests)

set(CMAKE_BUILD_TYPE Debug)

FILE(GLOB TEST_SOURCES *.cc *.cpp)

add_executable(runtests
	${TEST_SOURCES}
	doctest.cc
)


target_compile_features(runtests PUBLIC cxx_std_23)

target_include_directories(runtests PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/..
	${CMAKE_CURRENT_LIST_DIR}/../../cpputil
)


target_compile_definitions(runtests PRIVATE TESTPROJECT)

