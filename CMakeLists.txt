cmake_minimum_required(VERSION 3.10)
project(jiewu-project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(oatpp REQUIRED)
find_package(OpenSSL REQUIRED)

find_path(MYSQLCPPCONN_INCLUDE_DIR mysql_driver.h)
find_library(MYSQLCPPCONN_LIBRARY NAMES mysqlcppconn)

if(NOT MYSQLCPPCONN_INCLUDE_DIR)
    message(FATAL_ERROR "mysql_driver.h not found. Please install MySQL Connector/C++ development headers.")
endif()

if(NOT MYSQLCPPCONN_LIBRARY)
    message(FATAL_ERROR "mysqlcppconn library not found. Please install MySQL Connector/C++.")
endif()

add_executable(jiewu_server
    src/main.cpp
    src/db/UserRepository.cpp
    src/service/UserService.cpp
)

target_link_libraries(jiewu_server
    oatpp::oatpp
    OpenSSL::Crypto
    ${MYSQLCPPCONN_LIBRARY}
)

target_include_directories(jiewu_server PRIVATE
    src
    ${MYSQLCPPCONN_INCLUDE_DIR}
    /usr/local/include/oatpp-1.4.0/oatpp
)
