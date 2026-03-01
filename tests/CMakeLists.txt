# doctest as a header-only INTERFACE library from third_party/
add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE ${CMAKE_SOURCE_DIR}/third_party)

# Convenience function: one call per test source file
function(edfio_add_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE edfio doctest)
    if(MSVC)
        target_compile_options(${name} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${name} PRIVATE
            -Wall -Wextra -Wpedantic -Wno-c++98-compat
        )
    endif()
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# Copy test fixtures from files/ directory
configure_file(
    ${CMAKE_SOURCE_DIR}/files/Calib5.edf
    ${CMAKE_CURRENT_BINARY_DIR}/Calib5.edf
    COPYONLY
)
configure_file(
    ${CMAKE_SOURCE_DIR}/files/test_generator_2.edf
    ${CMAKE_CURRENT_BINARY_DIR}/test_generator_2.edf
    COPYONLY
)
configure_file(
    ${CMAKE_SOURCE_DIR}/files/test_generator_2.bdf
    ${CMAKE_CURRENT_BINARY_DIR}/test_generator_2.bdf
    COPYONLY
)
configure_file(
    ${CMAKE_SOURCE_DIR}/files/SC4001EC-Hypnogram.edf
    ${CMAKE_CURRENT_BINARY_DIR}/SC4001EC-Hypnogram.edf
    COPYONLY
)

# --- Test executables ---
edfio_add_test(test_record)
edfio_add_test(test_reader)
edfio_add_test(test_processor_sample)
edfio_add_test(test_iterators)
edfio_add_test(test_writer)
edfio_add_test(test_processor_utils)
edfio_add_test(test_bdf_reader)
edfio_add_test(test_processors)
edfio_add_test(test_edfplus)
edfio_add_test(test_edffile)
