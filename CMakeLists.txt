cmake_minimum_required(VERSION 4.0)
project(edfio VERSION 0.2.0 LANGUAGES CXX)

# ---- Header-only library target ----
add_library(edfio INTERFACE)
target_include_directories(edfio INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(edfio INTERFACE cxx_std_23)

# ---- Tests ----
option(EDFIO_BUILD_TESTS "Build tests" ON)
if(EDFIO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
