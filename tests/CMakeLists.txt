add_subdirectory(Catch2)
set_target_properties(Catch2 PROPERTIES CXX_STANDARD 17)

function(run_test source)
  get_filename_component(test_name ${source} NAME_WE)
  add_executable(${test_name} ${source})
  target_compile_features(${test_name} PRIVATE cxx_std_20)
  target_link_libraries(${test_name} soa.hpp Catch2::Catch2WithMain)
  add_test(NAME ${test_name} COMMAND ${test_name} ${ARGN})
endfunction()

run_test("soa_test.cpp")
run_test("readme_test.cpp")
