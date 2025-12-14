# Examples for edfio
cmake_minimum_required(VERSION 3.21)

# Helper function to create examples
function(add_example example_name)
    add_executable(${example_name} ${ARGN})
    target_link_libraries(${example_name} PRIVATE edfio::edfio)
    
    if(TARGET project_warnings)
        target_link_libraries(${example_name} PRIVATE project_warnings)
    endif()
    
    if(TARGET project_sanitizers)
        target_link_libraries(${example_name} PRIVATE project_sanitizers)
    endif()
endfunction()

# Add example executables
add_example(basic_usage basic_usage.cpp)
