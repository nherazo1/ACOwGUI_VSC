cmake_minimum_required(VERSION 3.5.0)
project(ACOWGUI_VSC VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_COMPILER "/usr/bin/g++" CACHE STRING "C compiler" FORCE)
set(CMAKE_C_COMPILER "/usr/bin/gcc" CACHE STRING "C++ compiler" FORCE)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(pybind11)
#add_subdirectory(pybind11/include/pybind11)
pybind11_add_module(module_name mainACO.cpp)
#add_executable(PYACOWGUI_VSC main.cpp)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
