# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD weact_studio_rp2350b_core CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(rp2350bULA C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(rp2350bULA ${CMAKE_CURRENT_LIST_DIR}/src/Main.c )

pico_generate_pio_header(rp2350bULA ${CMAKE_CURRENT_LIST_DIR}/src/MCLK.pio)
pico_generate_pio_header(rp2350bULA ${CMAKE_CURRENT_LIST_DIR}/src/CPUCLK.pio)
pico_generate_pio_header(rp2350bULA ${CMAKE_CURRENT_LIST_DIR}/src/IN_FE.pio)
pico_generate_pio_header(rp2350bULA ${CMAKE_CURRENT_LIST_DIR}/src/OUT_FE.pio)

pico_generate_pio_header(rp2350bULA ${CMAKE_CURRENT_LIST_DIR}/src/VGA.pio)
pico_generate_pio_header(rp2350bULA ${CMAKE_CURRENT_LIST_DIR}/src/activevideo.pio)
pico_generate_pio_header(rp2350bULA ${CMAKE_CURRENT_LIST_DIR}/src/VRAM_VGA.pio)

pico_generate_pio_header(rp2350bULA ${CMAKE_CURRENT_LIST_DIR}/src/INT.pio)
pico_generate_pio_header(rp2350bULA ${CMAKE_CURRENT_LIST_DIR}/src/Read_frame.pio)
pico_generate_pio_header(rp2350bULA ${CMAKE_CURRENT_LIST_DIR}/src/Read_line.pio)
pico_generate_pio_header(rp2350bULA ${CMAKE_CURRENT_LIST_DIR}/src/Read_VRAM.pio)

pico_set_program_name(rp2350bULA "rp2350bULA")
pico_set_program_version(rp2350bULA "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(rp2350bULA 0)
pico_enable_stdio_usb(rp2350bULA 0)

# Add the standard library to the build
target_link_libraries(rp2350bULA
        pico_stdlib
        hardware_pio
        hardware_dma
        pico_multicore
        )


# Add the standard include files to the build
target_include_directories(rp2350bULA PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src
)

pico_add_extra_outputs(rp2350bULA)

