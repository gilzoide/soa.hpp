add_subdirectory(Catch2)
set_target_properties(Catch2 PROPERTIES CXX_STANDARD 17)

add_executable(soa_test soa_test.cpp)
target_link_libraries(soa_test soa.hpp Catch2::Catch2WithMain)
set_target_properties(soa_test PROPERTIES CXX_STANDARD 20)

add_test(NAME test COMMAND soa_test)
