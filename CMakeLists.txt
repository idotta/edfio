cmake_minimum_required(VERSION 4.0)
project(edfio VERSION 0.2.0 LANGUAGES CXX)

# ---- Header-only library target ----
add_library(edfio INTERFACE)
target_include_directories(edfio INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(edfio INTERFACE cxx_std_23)
add_library(edfio::edfio ALIAS edfio)

# ---- Tests ----
option(EDFIO_BUILD_TESTS "Build tests" ON)
if(EDFIO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---- Install targets and packaging ----
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS edfio EXPORT edfioTargets)
install(DIRECTORY include/edfio DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT edfioTargets
    FILE edfioTargets.cmake
    NAMESPACE edfio::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edfio
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/edfioConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/edfioConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/edfioConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edfio
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/edfioConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/edfioConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/edfio
)
