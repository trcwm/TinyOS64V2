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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -fshort-wchar -nostartfiles -nostdlib -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -ffreestanding -Wl,-dll -shared -Wl,--subsystem,10 -e efi_init")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ffreestanding -nostartfiles -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_init -T${LINKERFILE}")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=output.map")

add_executable(boot
    src/allocator.cpp
    #src/commands.cpp
    src/pci.cpp
    #src/hdacodec.cpp
    src/myprint.cpp
    src/main.cpp)

add_custom_command(TARGET boot POST_BUILD
    COMMAND mv boot.exe BOOTX64.EFI
    COMMAND objdump -d BOOTX64.EFI >BOOTX64.asm
    COMMAND dd if=/dev/zero of=fat.img bs=1k count=1440
    COMMAND mformat -i fat.img -f 1440 ::
    COMMAND mmd -i fat.img ::/EFI
    COMMAND mmd -i fat.img ::/EFI/BOOT
    COMMAND mkdir -p ./dist/EFI/BOOT
    COMMAND mcopy -i fat.img BOOTX64.EFI ::/EFI/BOOT
    COMMAND mkgpt -o ${PROJECT_SOURCE_DIR}/hdimage.img --image-size 4096 --part fat.img --type system
)
