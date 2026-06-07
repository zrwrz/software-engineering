cmake_minimum_required(VERSION 3.10)
project(jiewu-project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Oat++ 库
find_package(oatpp REQUIRED)

# 添加可执行文件
add_executable(jiewu_server
    src/main.cpp
    # 后续添加其他 .cpp 文件
)

# 链接 Oat++ 库
target_link_libraries(jiewu_server
    oatpp::oatpp
)

# 设置头文件搜索路径
target_include_directories(jiewu_server PRIVATE
    src
    /usr/local/include/oatpp-1.4.0/oatpp
)