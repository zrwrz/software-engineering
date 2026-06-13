cmake_minimum_required(VERSION 3.10)
project(jiewu-project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)

# 1. oatpp
find_package(oatpp REQUIRED)

set(OATPP_INCLUDE_DIR "" CACHE PATH "oatpp include directory")

if(NOT EXISTS "${OATPP_INCLUDE_DIR}/oatpp/core/Types.hpp")
    if(DEFINED oatpp_DIR)
        get_filename_component(OATPP_INSTALL_PREFIX "${oatpp_DIR}/../../.." ABSOLUTE)
        find_path(OATPP_INCLUDE_DIR oatpp/core/Types.hpp
            PATHS
                "${OATPP_INSTALL_PREFIX}/include/oatpp-1.3.0/oatpp"
                "${OATPP_INSTALL_PREFIX}/include/oatpp-1.4.0/oatpp"
            NO_DEFAULT_PATH
        )
    endif()
endif()

if(NOT EXISTS "${OATPP_INCLUDE_DIR}/oatpp/core/Types.hpp")
    message(FATAL_ERROR "oatpp headers not found! Please set OATPP_INCLUDE_DIR manually.")
endif()

message(STATUS "Using oatpp include: ${OATPP_INCLUDE_DIR}")

# 2. OpenSSL
set(OPENSSL_ROOT_DIR "" CACHE PATH "OpenSSL root directory")
set(OPENSSL_INCLUDE_DIR "" CACHE PATH "OpenSSL include directory")
set(OPENSSL_SSL_LIBRARY "" CACHE FILEPATH "OpenSSL ssl library")
set(OPENSSL_CRYPTO_LIBRARY "" CACHE FILEPATH "OpenSSL crypto library")

if(EXISTS "${OPENSSL_INCLUDE_DIR}/openssl/ssl.h"
   AND EXISTS "${OPENSSL_SSL_LIBRARY}"
   AND EXISTS "${OPENSSL_CRYPTO_LIBRARY}")

    message(STATUS "Using manually specified OpenSSL libraries")

    add_library(OpenSSL::SSL UNKNOWN IMPORTED)
    set_target_properties(OpenSSL::SSL PROPERTIES
        IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
    )

    add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
    set_target_properties(OpenSSL::Crypto PROPERTIES
        IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
    )
else()
    find_package(OpenSSL REQUIRED)
endif()

# 3. MySQL Connector/C++
set(MYSQL_INCLUDE_DIR "" CACHE PATH "MySQL Connector/C++ include directory")
set(MYSQL_LIB "" CACHE FILEPATH "MySQL Connector/C++ library")

if(NOT EXISTS "${MYSQL_INCLUDE_DIR}/mysql_driver.h")
    find_path(MYSQL_INCLUDE_DIR mysql_driver.h
        PATHS
            "C:/Program Files/MySQL/MySQL Connector C++ 9.7/include/jdbc"
            "C:/Program Files/MySQL/Connector C++ 8.0/include/jdbc"
            "C:/Program Files/MySQL/MySQL Connector C++ 8.0/include/jdbc"
        NO_DEFAULT_PATH
    )
endif()

if(NOT EXISTS "${MYSQL_LIB}")
    find_library(MYSQL_LIB mysqlcppconn
        PATHS
            "C:/Program Files/MySQL/MySQL Connector C++ 9.7/lib64/vs14"
            "C:/Program Files/MySQL/Connector C++ 8.0/lib64/vs14"
            "C:/Program Files/MySQL/MySQL Connector C++ 8.0/lib64/vs14"
        NO_DEFAULT_PATH
    )
endif()

if(NOT EXISTS "${MYSQL_INCLUDE_DIR}/mysql_driver.h"
   OR NOT EXISTS "${MYSQL_INCLUDE_DIR}/cppconn/connection.h"
   OR NOT EXISTS "${MYSQL_LIB}")
    message(FATAL_ERROR "MySQL Connector/C++ not found! Please set MYSQL_INCLUDE_DIR and MYSQL_LIB manually.")
endif()

set(MYSQLCPPCONN_INCLUDE_DIR "${MYSQL_INCLUDE_DIR}" CACHE PATH "MySQL Connector/C++ include directory" FORCE)
set(MYSQLCPPCONN_LIBRARY "${MYSQL_LIB}" CACHE FILEPATH "MySQL Connector/C++ library" FORCE)

message(STATUS "Using MySQL include: ${MYSQLCPPCONN_INCLUDE_DIR}")
message(STATUS "Using MySQL lib: ${MYSQLCPPCONN_LIBRARY}")

# 4. 源文件
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(jiewu_server ${PROJECT_SOURCES})

# 5. 头文件目录
target_include_directories(jiewu_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OATPP_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${MYSQLCPPCONN_INCLUDE_DIR}
)

# 6. 链接库
target_link_libraries(jiewu_server PRIVATE
    oatpp::oatpp
    OpenSSL::SSL
    OpenSSL::Crypto
    "${MYSQLCPPCONN_LIBRARY}"
    crypt32
    ws2_32
    bcrypt
    advapi32
    user32
)