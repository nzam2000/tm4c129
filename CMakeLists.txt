cmake_minimum_required(VERSION 3.28)
project(tm4c129 C)
set(CMAKE_C_STANDARD 11)

add_compile_options(-march=armv7e-m -g3)
add_link_options(-T ${PROJECT_SOURCE_DIR}/gcc_tm4c129.ld -nostdlib -nostartfiles -nodefaultlibs -Wl,-Map=tm4c129_map.txt)

set(SOURCES
        src/main.c
        src/startup_tm4c129.c
        src/stdlib/printf.c
        src/stdlib/stdlib_hooks.c
)

include_directories(
        src
)

add_executable(tm4c129 ${SOURCES})

add_custom_command(TARGET tm4c129 POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary tm4c129 tm4c129.bin
        COMMAND ${CMAKE_OBJDUMP} -d -z --insn-width=4 -EL tm4c129 > tm4c129_disassembly.txt
)

set_property(
        TARGET tm4c129
        APPEND
        PROPERTY ADDITIONAL_CLEAN_FILES tm4c129.bin tm4c129_map.txt tm4c129_disassembly.txt
)