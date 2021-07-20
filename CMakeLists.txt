cmake_minimum_required(VERSION 3.16)
project(boot)

set(CMAKE_CXX_STANDARD 17)

set(tools /usr/bin)
set(CMAKE_C_COMPILER ${tools}/x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER ${tools}/x86_64-w64-mingw32-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(LINKERFILE ${PROJECT_SOURCE_DIR}/linker/linker.ld)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main")

add_executable(boot src/main.cpp)

add_custom_command(TARGET boot POST_BUILD
    COMMAND mv boot.exe BOOTX64.EFI
    COMMAND dd if=/dev/zero of=fat.img bs=1k count=1440
    COMMAND mformat -i fat.img -f 1440 ::
    COMMAND mmd -i fat.img ::/EFI
    COMMAND mmd -i fat.img ::/EFI/BOOT
    COMMAND mkdir -p ./dist/EFI/BOOT
    COMMAND mcopy -i fat.img BOOTX64.EFI ::/EFI/BOOT
    COMMAND mkgpt -o ${PROJECT_SOURCE_DIR}/hdimage.img --image-size 4096 --part fat.img --type system
)
