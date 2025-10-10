cmake_minimum_required(VERSION 3.19)

project(soa.hpp)

option(SOA_BUILD_TESTS "Whether to build automated tests" OFF)

add_library(soa.hpp INTERFACE soa.hpp)
target_compile_features(soa.hpp INTERFACE cxx_std_20)
target_include_directories(soa.hpp INTERFACE .)

if(SOA_BUILD_TESTS)
  include(CTest)
  add_subdirectory(tests)
endif()
