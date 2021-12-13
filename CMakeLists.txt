cmake_minimum_required(VERSION 3.20)
project(DS1307 C)

set(CMAKE_C_STANDARD 99)

include(cmake/CPM.cmake)

CPMAddPackage(
        NAME STM32Core
        GITHUB_REPOSITORY ximtech/STM32Core
        GIT_TAG origin/main)

set(DS1307_DIRECTORY
        ${I2C_POLLING_DIRECTORY}
        ${DS1307_SOURCE_DIR}
        ${DS1307_SOURCE_DIR}/include
        CACHE STRING "DS1307 directories include to the main project" FORCE)

set(DS1307_SOURCES
        ${I2C_POLLING_SOURCES}
        ${DS1307_SOURCE_DIR}/${PROJECT_NAME}.c
        ${DS1307_SOURCE_DIR}/include/${PROJECT_NAME}.h
        CACHE STRING "DS1307 source files include to the main project" FORCE)