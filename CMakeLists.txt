# ThaiOS Master CMakeLists.txt
# ============================
cmake_minimum_required(VERSION 3.25)
project(ThaiOS VERSION 0.1.0 LANGUAGES C CXX ASM)

# === Architecture detection ===
if(NOT DEFINED ARCH)
    set(ARCH "x86_64" CACHE STRING "Target architecture")
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -Wall -Wextra -O2 -fno-stack-protector")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -f elf64")

# === Architecture-specific flags ===
if(${ARCH} STREQUAL "x86_64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel")
elseif(${ARCH} STREQUAL "aarch64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a -mcmodel=large")
endif()

# === Subdirectories ===
add_subdirectory(kernel)
add_subdirectory(lib)
add_subdirectory(drivers)
add_subdirectory(fs)
add_subdirectory(net)
add_subdirectory(security)
add_subdirectory(gui)
add_subdirectory(apps)
add_subdirectory(tools)

# === Final link ===
set(KERNEL_OBJECTS
    ${KERNEL_OBJS}
    ${LIB_OBJS}
    ${DRIVER_OBJS}
    ${FS_OBJS}
    ${NET_OBJS}
    ${SECURITY_OBJS}
)

add_executable(kernel.elf ${KERNEL_OBJECTS})
target_link_options(kernel.elf PRIVATE
    -T ${CMAKE_SOURCE_DIR}/kernel/arch/${ARCH}/linker.ld
    -nostdlib
    -z max-page-size=0x1000
)
