cmake_minimum_required(VERSION 3.26)
project(Ctrl-Lib C)

set(CMAKE_C_STANDARD 11)

set(CTRL_SDK_INC_PATH ctrl-sdk/ ctrl-sdk/cfg ctrl-sdk/inc)
include_directories(${CTRL_SDK_INC_PATH})

file(GLOB_RECURSE CTRL_SDK_SOURCES "ctrl-sdk/*.*")

add_library(Ctrl-Lib ${CTRL_SDK_SOURCES})
