# 指定最低版本
cmake_minimum_required(VERSION 3.0)

# 项目名称
project(WebServer)

# 设置C++
set(CMAKE_CXX_STANDARD 11)

# 搜索源文件
aux_source_directory(${PROJECT_SOURCE_DIR} SRC)

# 移除server.cpp
list(REMOVE_ITEM SRC ${PROJECT_SOURCE_DIR}/server.cpp)

# 指定生成目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 查找线程库
find_package(Threads REQUIRED)

# include_directories(/usr/include/mysql)  
# link_directories(/usr/lib/mysql)  
# 生成可执行文件
add_executable(server ${SRC})

# 生成调试文件
target_compile_options(server PRIVATE -g)

# 需要链接的动态库 1线程库 2数据库
target_link_libraries(server  Threads::Threads mysqlclient)