CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)
project(MAIX_LOADER)

set(CMAKE_VERBOSE_MAKEFILE ON)

if(NOT WIN32)
    string(ASCII 27 Esc)
    set(ColourReset "${Esc}[m")
    set(ColourBold  "${Esc}[1m")
    set(Red         "${Esc}[31m")
    set(Green       "${Esc}[32m")
    set(Yellow      "${Esc}[33m")
    set(Blue        "${Esc}[34m")
    set(Magenta     "${Esc}[35m")
    set(Cyan        "${Esc}[36m")
    set(White       "${Esc}[37m")
    set(BoldRed     "${Esc}[1;31m")
    set(BoldGreen   "${Esc}[1;32m")
    set(BoldYellow  "${Esc}[1;33m")
    set(BoldBlue    "${Esc}[1;34m")
    set(BoldMagenta "${Esc}[1;35m")
    set(BoldCyan    "${Esc}[1;36m")
    set(BoldWhite   "${Esc}[1;37m")
endif()

IF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt.")
ENDIF()

message(STATUS "Check for RISCV toolchain ...")
IF("${TOOLCHAIN}" STREQUAL "")
    message(STATUS "Using default RISCV toolchain")
    SET(CMAKE_C_COMPILER "riscv64-unknown-elf-gcc")
    SET(CMAKE_CXX_COMPILER  "riscv64-unknown-elf-g++")
    SET(CMAKE_LINKER "riscv64-unknown-elf-ld" CACHE INTERNAL "")
    SET(CMAKE_AR "riscv64-unknown-elf-ar" CACHE INTERNAL "")
    SET(CMAKE_OBJCOPY "riscv64-unknown-elf-objcopy")
    SET(CMAKE_SIZE "riscv64-unknown-elf-size")
    SET(CMAKE_OBJDUMP "riscv64-unknown-elf-objdump")
ELSE()
    message(STATUS "Using ${TOOLCHAIN} RISCV toolchain")
    SET(CMAKE_C_COMPILER "${TOOLCHAIN}/riscv64-unknown-elf-gcc")
    SET(CMAKE_CXX_COMPILER  "${TOOLCHAIN}/riscv64-unknown-elf-g++")
    SET(CMAKE_LINKER "${TOOLCHAIN}/riscv64-unknown-elf-ld" CACHE INTERNAL "")
    SET(CMAKE_AR "${TOOLCHAIN}/riscv64-unknown-elf-ar" CACHE INTERNAL "")
    SET(CMAKE_OBJCOPY "${TOOLCHAIN}/riscv64-unknown-elf-objcopy")
    SET(CMAKE_SIZE "${TOOLCHAIN}/riscv64-unknown-elf-size")
    SET(CMAKE_OBJDUMP "${TOOLCHAIN}/riscv64-unknown-elf-objdump")
ENDIF()

execute_process(COMMAND sh "-c" "${CMAKE_C_COMPILER} -dumpversion" RESULT_VARIABLE GCC_RESULT OUTPUT_VARIABLE GCC_INFO)
string(STRIP ${GCC_INFO} GCC_INFO)
message(STATUS "Check for RISCV GCC ...")
IF(${GCC_RESULT} EQUAL 0})
    message(STATUS "Using RISCV GCC: ${GCC_INFO}")
ELSE()
    message(FATAL_ERROR "Can not found RISCV GCC, please install and check")
ENDIF()

execute_process(COMMAND sh "-c" "python -c \"exec('import platform\\nprint(platform.python_version())')\"" RESULT_VARIABLE PYTHON_RESULT OUTPUT_VARIABLE PYTHON_INFO)
string(STRIP ${PYTHON_INFO} PYTHON_INFO)
message(STATUS "Check for python ...")
IF(${PYTHON_RESULT} EQUAL 0})
    message(STATUS "Using python: ${PYTHON_INFO}")
ELSE()
    message(FATAL_ERROR "Can not found python, please install and check")
ENDIF()

execute_process(COMMAND sh "-c" "git --version" RESULT_VARIABLE GIT_RESULT OUTPUT_VARIABLE GIT_INFO)
string(STRIP ${GIT_INFO} GIT_INFO)
message(STATUS "Check for git ...")
IF(${GIT_RESULT} EQUAL 0})
    message(STATUS "Using git: ${GIT_INFO}")
ELSE()
    message(FATAL_ERROR "Can not found git, please install and check")
ENDIF()

IF(NOT CMAKE_BUILD_TYPE)
    SET( CMAKE_BUILD_TYPE Release)
ELSE()
    IF ((NOT CMAKE_BUILD_TYPE STREQUAL "Debug") AND (NOT CMAKE_BUILD_TYPE STREQUAL "Release"))
        message(FATAL_ERROR "CMAKE_BUILD_TYPE must either be Debug or Release instead of ${CMAKE_BUILD_TYPE}")
    ENDIF()
ENDIF()

message(STATUS "Check build type: CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")



# executable config

#ADD_SUBDIRECTORY(src)
INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(src/include)

file(GLOB_RECURSE DIR_SRCS src src/*.S src/*.s src/*.c)
set_property(SOURCE src/crt.S PROPERTY LANGUAGE C)
set_source_files_properties(src/crt.S PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp -D __riscv64")



# Debug & Release
IF(CMAKE_BUILD_TYPE STREQUAL Debug)
    message(WARNING "${Red}THIS IS A DEBUG BUILD, DO NOT USE IT IN PRODUCTION!!!${ColourReset}")
    add_definitions(-DDEBUG=1)
ELSE()
    message("${Green}[PRODUCTION CHECK] CMAKE_BUILD_TYPE = Release ............ OK ${ColourReset}")
ENDIF()



# C Flags Settings

set(COMMON_FLAGS "-std=gnu99 \
-mcmodel=medany \
-fno-common \
-ffunction-sections \
-fdata-sections \
-fstrict-volatile-bitfields \
-march=rv64imafd")

set(LDFLAGS "\
-nostdlib               \
-nostartfiles           \
-static                 \
-Wl,--gc-sections       \
-Wl,--print-gc-sections \
-Wl,-static             \
-Wl,-EL")

set(COMMON_WARNING_FLAGS "-Wall \
-Werror=all \
-Wno-error=unused-function \
-Wno-error=unused-but-set-variable \
-Wno-error=unused-variable \
-Wno-error=deprecated-declarations \
-Wextra \
-Wcast-align \
-Werror=frame-larger-than=65536 \
-Wno-unused-parameter \
-Wno-sign-compare \
-Wno-old-style-declaration \
${COMMON_FLAGS}")

SET(CMAKE_C_FLAGS "${COMMON_WARNING_FLAGS}")
STRING(REPLACE "-O3" "-O0" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
message(STATUS "CMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}")

# definitions in macros
add_definitions(-DCONFIG_LOG_LEVEL=LOG_ERROR)

execute_process(COMMAND sh "-c" "git rev-parse HEAD | cut -c 1-8" RESULT_VARIABLE GIT_RESULT OUTPUT_VARIABLE GIT_INFO)
string(STRIP ${GIT_INFO} GIT_INFO)
message(STATUS "Build with git commit: ${GIT_INFO}")
add_definitions(-DCONFIG_GIT_ID=\"${GIT_INFO}\")
add_definitions(-DBUILD_VERSION=\"${GIT_INFO}\")



add_executable(loader_stage1 ${DIR_SRCS} src/include/common.h)
target_compile_definitions(loader_stage1 PUBLIC LOADER_STAGE1=1)
target_link_libraries(loader_stage1 -Wl,--start-group c -Wl,--end-group ${LDFLAGS} -Xlinker --defsym=LOADER_STAGE1=1 -L${CMAKE_CURRENT_SOURCE_DIR}/ld -T${CMAKE_CURRENT_SOURCE_DIR}/ld/maixloader.ld)

add_executable(loader_stage2 ${DIR_SRCS} src/include/common.h)
target_compile_definitions(loader_stage2 PUBLIC LOADER_STAGE2=1)
target_link_libraries(loader_stage2 -Wl,--start-group c -Wl,--end-group ${LDFLAGS} -Xlinker --defsym=LOADER_STAGE2=1 -L${CMAKE_CURRENT_SOURCE_DIR}/ld -T${CMAKE_CURRENT_SOURCE_DIR}/ld/maixloader.ld)



# generate bin file

add_custom_target(build_loaders
        DEPENDS loader_stage1 loader_stage2
        COMMENT "Building loader_stage 1 & 2 ..."
        )

add_custom_command(TARGET loader_stage1 POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --output-format=binary ${CMAKE_BINARY_DIR}/loader_stage1 ${CMAKE_BINARY_DIR}/loader_stage1.bin
        COMMAND ${CMAKE_SIZE} ${CMAKE_BINARY_DIR}/loader_stage1
        COMMAND ${CMAKE_OBJDUMP} -lSD ${CMAKE_BINARY_DIR}/loader_stage1 > ${CMAKE_BINARY_DIR}/loader_stage1.S
        COMMAND python2 ../utils/adjhex.py < ${CMAKE_BINARY_DIR}/loader_stage1.bin > ${CMAKE_BINARY_DIR}/loader_stage1.mem
        COMMAND python2 ../utils/adjhex.py --ecc < ${CMAKE_BINARY_DIR}/loader_stage1.bin > ${CMAKE_BINARY_DIR}/loader_stage1.ecc.mem
        COMMAND python2 ../utils/memmixer.py < ${CMAKE_BINARY_DIR}/loader_stage1.ecc.mem > ${CMAKE_BINARY_DIR}/loader_stage1.mix.mem
        DEPENDS loader_stage1
        COMMENT "Generating .bin and .mix.mem file for loader_stage1 ...")

add_custom_command(TARGET loader_stage2 POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --output-format=binary ${CMAKE_BINARY_DIR}/loader_stage2 ${CMAKE_BINARY_DIR}/loader_stage2.bin
        COMMAND ${CMAKE_SIZE} ${CMAKE_BINARY_DIR}/loader_stage2
        COMMAND ${CMAKE_OBJDUMP} -lSD ${CMAKE_BINARY_DIR}/loader_stage2 > ${CMAKE_BINARY_DIR}/loader_stage2.S
        COMMAND python2 ../utils/adjhex.py < ${CMAKE_BINARY_DIR}/loader_stage2.bin > ${CMAKE_BINARY_DIR}/loader_stage2.mem
        COMMAND python2 ../utils/adjhex.py --ecc < ${CMAKE_BINARY_DIR}/loader_stage2.bin > ${CMAKE_BINARY_DIR}/loader_stage2.ecc.mem
        COMMAND python2 ../utils/memmixer.py < ${CMAKE_BINARY_DIR}/loader_stage2.ecc.mem > ${CMAKE_BINARY_DIR}/loader_stage2.mix.mem
        COMMAND python2 ../utils/genimg.py ${CMAKE_BINARY_DIR}/loader_stage1.bin ${CMAKE_BINARY_DIR}/loader_stage2.bin ${CMAKE_BINARY_DIR}/loader.img
        DEPENDS loader_stage2
        COMMENT "Generating .bin and .mix.mem file for loader_stage2 ...")

set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
        "${CMAKE_BINARY_DIR}/loader_stage1.bin"
        "${CMAKE_BINARY_DIR}/loader_stage1.S"
        "${CMAKE_BINARY_DIR}/loader_stage1.mem"
        "${CMAKE_BINARY_DIR}/loader_stage1.ecc.mem"
        "${CMAKE_BINARY_DIR}/loader_stage1.mix.mem"
        "${CMAKE_BINARY_DIR}/loader_stage2.bin"
        "${CMAKE_BINARY_DIR}/loader_stage2.S"
        "${CMAKE_BINARY_DIR}/loader_stage2.mem"
        "${CMAKE_BINARY_DIR}/loader_stage2.ecc.mem"
        "${CMAKE_BINARY_DIR}/loader_stage2.mix.mem")

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
