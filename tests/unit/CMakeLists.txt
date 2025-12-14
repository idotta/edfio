# Unit tests for edfio
cmake_minimum_required(VERSION 3.21)

# Helper function to create unit tests
function(add_unit_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} PRIVATE test_utilities)
    gtest_discover_tests(${test_name}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        PROPERTIES
            LABELS "unit"
    )
endfunction()

# Add unit test executables
add_unit_test(test_config test_config.cpp)
add_unit_test(test_data_format test_data_format.cpp)
add_unit_test(test_sample_type test_sample_type.cpp)
