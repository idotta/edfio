cmake_minimum_required(VERSION 3.21)

# Project definition
project(edfio
    VERSION 1.0.0
    DESCRIPTION "A C++20 header-only library to read/write EDF(+)/BDF(+) files"
    LANGUAGES CXX
)

# Only build tests if this is the main project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(EDFIO_IS_TOPLEVEL_PROJECT TRUE)
else()
    set(EDFIO_IS_TOPLEVEL_PROJECT FALSE)
endif()

# Options
option(EDFIO_BUILD_TESTS "Build tests" ${EDFIO_IS_TOPLEVEL_PROJECT})
option(EDFIO_BUILD_EXAMPLES "Build examples" ${EDFIO_IS_TOPLEVEL_PROJECT})
option(EDFIO_ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(EDFIO_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
option(EDFIO_ENABLE_CPPCHECK "Enable cppcheck" OFF)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define the header-only library
add_library(edfio INTERFACE)
add_library(edfio::edfio ALIAS edfio)

# Specify include directories
target_include_directories(edfio
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Require C++20
target_compile_features(edfio INTERFACE cxx_std_20)

# Add compiler warnings
if(EDFIO_IS_TOPLEVEL_PROJECT)
    # Define warning flags for different compilers
    set(MSVC_WARNINGS
        /W4          # Baseline reasonable warnings
        /w14242      # 'identifier': conversion from 'type1' to 'type2', possible loss of data
        /w14254      # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
        /w14263      # 'function': member function does not override any base class virtual member function
        /w14265      # 'classname': class has virtual functions, but destructor is not virtual
        /w14287      # 'operator': unsigned/negative constant mismatch
        /we4289      # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
        /w14296      # 'operator': expression is always 'boolean_value'
        /w14311      # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545      # expression before comma evaluates to a function which is missing an argument list
        /w14546      # function call before comma missing argument list
        /w14547      # 'operator': operator before comma has no effect; expected operator with side-effect
        /w14549      # 'operator': operator before comma has no effect; did you intend 'operator'?
        /w14555      # expression has no effect; expected expression with side-effect
        /w14619      # pragma warning: there is no warning number 'number'
        /w14640      # Enable warning on thread un-safe static member initialization
        /w14826      # Conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior.
        /w14905      # wide string literal cast to 'LPSTR'
        /w14906      # string literal cast to 'LPWSTR'
        /w14928      # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
        /permissive- # standards conformance mode
    )

    set(CLANG_WARNINGS
        -Wall
        -Wextra
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
    )

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    )

    # Set warnings based on compiler
    if(MSVC)
        set(PROJECT_WARNINGS ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(PROJECT_WARNINGS ${GCC_WARNINGS})
    else()
        message(WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
    endif()

    # Create an interface library for project warnings
    add_library(project_warnings INTERFACE)
    target_compile_options(project_warnings INTERFACE ${PROJECT_WARNINGS})
endif()

# Sanitizers
if(EDFIO_ENABLE_SANITIZERS)
    include(cmake/Sanitizers.cmake)
    add_library(project_sanitizers INTERFACE)
    enable_sanitizers(project_sanitizers)
endif()

# Static analyzers
if(EDFIO_ENABLE_CLANG_TIDY)
    find_program(CLANGTIDY clang-tidy)
    if(CLANGTIDY)
        set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY} -extra-arg=-Wno-unknown-warning-option)
        message(STATUS "clang-tidy enabled")
    else()
        message(WARNING "clang-tidy requested but executable not found")
    endif()
endif()

if(EDFIO_ENABLE_CPPCHECK)
    find_program(CPPCHECK cppcheck)
    if(CPPCHECK)
        set(CMAKE_CXX_CPPCHECK
            ${CPPCHECK}
            --suppress=missingInclude
            --enable=all
            --inline-suppr
            --inconclusive
        )
        message(STATUS "cppcheck enabled")
    else()
        message(WARNING "cppcheck requested but executable not found")
    endif()
endif()

# Testing
if(EDFIO_BUILD_TESTS)
    enable_testing()
    include(CTest)
    add_subdirectory(tests)
endif()

# Examples
if(EDFIO_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation rules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS edfio
    EXPORT edfioTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT edfioTargets
    FILE edfioTargets.cmake
    NAMESPACE edfio::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edfio
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/edfioConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/edfioConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edfio
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/edfioConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/edfioConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/edfioConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edfio
)

# Export from the build tree
export(EXPORT edfioTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/edfioTargets.cmake
    NAMESPACE edfio::
)
