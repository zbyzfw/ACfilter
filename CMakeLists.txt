cmake_minimum_required(VERSION 3.19)
#add_compile_options(-std=c++11)
set(CMAKE_CXX_STANDARD 14)
project(filter LANGUAGES CXX)

#add_executable(filter main.cpp)
include_directories(pybind11-master/include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# python文件夹
#set(PYTHON3_INC_DIR "C:/ProgramData/Miniconda3/include/python" CACHE PATH "")
set(PYTHON3_INC_DIR "D:/Miniconda3/include/python" CACHE PATH "")
#set(PYTHON_EXECUTABLE "C:/ProgramData/Miniconda3/python")
set(PYTHON_EXECUTABLE "D:/Miniconda3/python")

# 增加子文件夹,cmake会自动调用其中的cmakelists文件
add_subdirectory(${CMAKE_SOURCE_DIR}/pybind11-master)

# pybind11内部函数
pybind11_add_module(example example.cpp)

target_include_directories(example PRIVATE ${PYTHON3_INC_DIR})

