#..
# cmake_minimum_required(VERSION 3.0)
# project(main)

# # 配置编译选项,  -o  -g  
# # set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g -O2)
# set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
# add_compile_options(-std=c++14)

# 配置头文件搜索路径, 
# include_directories(.)

# 配置库文件搜索路径,
# link_directories()

# 设置可执行文件最终存储的路径,
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
message("EXECUTABLE_OUTPUT_PATH=${PROJECT_SOURCE_DIR}/bin")


# 设置需要编译的源文件列表
set(SRC_LIST  muduo_server.cpp)
# aux_source_directory(. SRC_LIST)  把指定路径下的所有源文件名字放入变量名 SRC_LIST 里面,

# 生成可执行文件 server, 由  ${SRC_LIST} 所有定义的源文件编译而来,
add_executable(server ${SRC_LIST})

# 生成库文件,
# add_library()

# 表示server这个目标程序需要链接这三个库文件,
target_link_libraries(server  muduo_net muduo_base pthread)

# 
# 


