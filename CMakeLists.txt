cmake_minimum_required(VERSION 3.26)
project(srv_sleep_sig_11)

set(CMAKE_CXX_STANDARD 17)

set(INCLUDE_DIRECTORIES
        .
        /opt/sap/OCS-16_0/include
)
include_directories(${INCLUDE_DIRECTORIES})

set(SOURCE_FILES
        example.h
        exutils.c
        exutils.h
        ossample.h
        srv_sleep_sig_11.c
        utils.c
        srv_sleep_sig_11.h
)

add_executable(srv_sleep_sig_11 ${SOURCE_FILES})

target_link_libraries(srv_sleep_sig_11
        sybsrv64 sybct64 sybtcl64 sybcs64 sybcomn64 sybintl64 sybunic64
)

set_target_properties(srv_sleep_sig_11
        PROPERTIES LINK_FLAGS
        -L/opt/sap/OCS-16_0/lib
)

target_compile_options(srv_sleep_sig_11 PRIVATE -m64 PRIVATE -g PRIVATE -DSYB_LP64 PRIVATE -D_REENTRANT PRIVATE -Werror PRIVATE -Wall PRIVATE -Wformat=2)

set(CMAKE_VERBOSE_MAKEFILE ON)
