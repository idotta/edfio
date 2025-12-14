# Sanitizers configuration for CMake
# Supports Address, Undefined Behavior, Thread, Memory, and Leak sanitizers

function(enable_sanitizers target_name)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" TRUE)
        option(ENABLE_SANITIZER_LEAK "Enable leak sanitizer" FALSE)
        option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR "Enable undefined behavior sanitizer" TRUE)
        option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" FALSE)
        option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" FALSE)

        set(SANITIZERS "")

        if(ENABLE_SANITIZER_ADDRESS)
            list(APPEND SANITIZERS "address")
        endif()

        if(ENABLE_SANITIZER_LEAK)
            list(APPEND SANITIZERS "leak")
        endif()

        if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
            list(APPEND SANITIZERS "undefined")
        endif()

        if(ENABLE_SANITIZER_THREAD)
            if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
                message(WARNING "Thread sanitizer is incompatible with Address and Leak sanitizers. Disabling Thread sanitizer.")
            else()
                list(APPEND SANITIZERS "thread")
            endif()
        endif()

        if(ENABLE_SANITIZER_MEMORY)
            if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS OR "thread" IN_LIST SANITIZERS)
                message(WARNING "Memory sanitizer is incompatible with Address, Leak, and Thread sanitizers. Disabling Memory sanitizer.")
            else()
                list(APPEND SANITIZERS "memory")
            endif()
        endif()

        list(JOIN SANITIZERS "," SANITIZERS_LIST)

        if(SANITIZERS_LIST)
            if(NOT "${SANITIZERS_LIST}" STREQUAL "")
                target_compile_options(${target_name} INTERFACE
                    -fsanitize=${SANITIZERS_LIST}
                    -fno-omit-frame-pointer
                    -fno-optimize-sibling-calls
                )
                target_link_options(${target_name} INTERFACE
                    -fsanitize=${SANITIZERS_LIST}
                )
                message(STATUS "Sanitizers enabled: ${SANITIZERS_LIST}")
            endif()
        endif()
    elseif(MSVC)
        if(ENABLE_SANITIZER_ADDRESS)
            target_compile_options(${target_name} INTERFACE /fsanitize=address)
            message(STATUS "Address sanitizer enabled for MSVC")
        endif()
    endif()
endfunction()
