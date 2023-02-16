cmake_minimum_required(VERSION 3.18)

project(iFoot3D LANGUAGES C CXX)

option(STATIC_WINDOWS_RUNTIME "Use static (MT/MTd) Windows runtime" ON)
if(STATIC_WINDOWS_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE main.cpp)

set(Open3D_ROOT C:/Open3D)
find_package(Open3D REQUIRED)

# Pipeline library
add_subdirectory(pipeline)

include_directories(pipeline)
target_link_directories(${PROJECT_NAME} PRIVATE pipeline/)

target_link_libraries(${PROJECT_NAME} PRIVATE Open3D::Open3D pipeline)
