cmake_minimum_required(VERSION 3.10)
project(jiewu-project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 解决可能的 ABI 问题
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)

# 1. 查找依赖包
find_package(oatpp REQUIRED)
find_package(OpenSSL REQUIRED)

# 2. 查找 MySQL C++ 连接器
find_path(MYSQLCPPCONN_INCLUDE_DIR mysql_driver.h)
find_library(MYSQLCPPCONN_LIBRARY NAMES mysqlcppconn mysqlcppconn-static)

if(NOT MYSQLCPPCONN_INCLUDE_DIR OR NOT MYSQLCPPCONN_LIBRARY)
    message(FATAL_ERROR "MySQL Connector/C++ not found! Please install libmysqlcppconn-dev.")
endif()

message(STATUS "Found MySQL Include: ${MYSQLCPPCONN_INCLUDE_DIR}")
message(STATUS "Found MySQL Library: ${MYSQLCPPCONN_LIBRARY}")

# 3. 定义源文件列表
# 编译 src 下全部 cpp
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

# 4. 添加可执行文件
add_executable(jiewu_server ${PROJECT_SOURCES})

# 5. 链接库文件
target_link_libraries(jiewu_server PRIVATE
    oatpp::oatpp
    OpenSSL::SSL
    OpenSSL::Crypto
    ${MYSQLCPPCONN_LIBRARY}
)

# 6. 设置头文件搜索路径
target_include_directories(jiewu_server PRIVATE
    src
    ${MYSQLCPPCONN_INCLUDE_DIR}
    /usr/local/include/oatpp-1.4.0/oatpp
)
