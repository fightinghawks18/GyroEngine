cmake_minimum_required(VERSION 3.10)
project(GyroEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GyroBuildDir ${CMAKE_BINARY_DIR}/Gyro)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GyroBuildDir})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GyroBuildDir})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GyroBuildDir})

add_subdirectory(apps)
add_subdirectory(src)