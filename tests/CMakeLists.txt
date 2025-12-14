# Test configuration for edfio
cmake_minimum_required(VERSION 3.21)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()
include(GoogleTest)

# Create a test utilities library
add_library(test_utilities INTERFACE)
target_link_libraries(test_utilities
    INTERFACE
        edfio::edfio
        GTest::gtest
        GTest::gtest_main
)

if(TARGET project_warnings)
    target_link_libraries(test_utilities INTERFACE project_warnings)
endif()

if(TARGET project_sanitizers)
    target_link_libraries(test_utilities INTERFACE project_sanitizers)
endif()

# Add unit tests
add_subdirectory(unit)

# Add integration tests
add_subdirectory(integration)
