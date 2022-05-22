# 设置cmake的最低版本和项目名称
cmake_minimum_required(VERSION 3.0)
project(mprpc)

# 配置编译选项,  -o  -g  
# set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g -O2)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
add_compile_options(-std=c++14)

# 生成 debug 版本,可以进行 gdb 调试,
set(CMAKE_BUILD_TYPE "Debug")

# 配置最终的可执行文件的输出路径,
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 配置项目头文件的搜索路径,
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

# 设置项目库文件搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 增加 CMakeLists 子目录
# src 包含了 mprpc 框架的相关代码
add_subdirectory(src)
# example 包含了 mprpc 框架的使用示例代码,
add_subdirectory(example)


