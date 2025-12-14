# Integration tests for edfio
cmake_minimum_required(VERSION 3.21)

# Helper function to create integration tests
function(add_integration_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} PRIVATE test_utilities)
    gtest_discover_tests(${test_name}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PROPERTIES
            LABELS "integration"
    )
endfunction()

# Add integration test executables
add_integration_test(test_edf_read test_edf_read.cpp)
