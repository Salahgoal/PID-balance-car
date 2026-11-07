cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME BalanceCar)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# --------------------------
# 【关键修改】删除了重复的 system_stm32f1xx.c
# --------------------------
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # 自动生成的文件（无需修改，CubeMX会维护）
    Core/Src/main.c
    Core/Src/gpio.c
    Core/Src/adc.c
    Core/Src/tim.c
    Core/Src/stm32f1xx_it.c
    Core/Src/stm32f1xx_hal_msp.c
    Core/Src/syscalls.c
    Core/Src/sysmem.c

    # 👇 你自己的驱动文件，全部在这里添加
    Core/Src/Battery.c
    Core/Src/dwt.c
    Core/Src/encoder.c
    Core/Src/hcsr04.c
    Core/Src/motor.c
    Core/Src/oled.c
)

# --------------------------
# 【关键修改2】添加头文件目录
# --------------------------
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # 包含自定义头文件的目录
    ${CMAKE_SOURCE_DIR}/Core/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)