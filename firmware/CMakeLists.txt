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
        src/uart/uart_comm.c
)

include_directories(
        src
        src/uart
)

add_executable(${PROJECT_NAME} ${SOURCES})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME} ${PROJECT_NAME}.bin
        COMMAND ${CMAKE_OBJDUMP} -d -z --insn-width=4 -EL ${PROJECT_NAME} > ${PROJECT_NAME}_disassembly.txt
)

set_property(
        TARGET ${PROJECT_NAME}
        APPEND
        PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_NAME}.bin ${PROJECT_NAME}_map.txt ${PROJECT_NAME}_disassembly.txt
)