cmake_minimum_required(VERSION 3.10)
project(jiewu-project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)

# macOS(Homebrew) 默认路径提示（仍可被命令行 -D 覆盖）
if(APPLE)
    if(NOT DEFINED oatpp_DIR AND EXISTS "/opt/homebrew/opt/oatpp/lib/cmake/oatpp-1.3.1/oatppConfig.cmake")
        set(oatpp_DIR "/opt/homebrew/opt/oatpp/lib/cmake/oatpp-1.3.1" CACHE PATH "oatpp config directory")
    endif()
    if((NOT DEFINED OPENSSL_ROOT_DIR OR OPENSSL_ROOT_DIR STREQUAL "") AND EXISTS "/opt/homebrew/opt/openssl@3")
        set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl@3" CACHE PATH "OpenSSL root directory")
    endif()
endif()

# nlohmann/json (used by jwt helper)
find_package(nlohmann_json QUIET)
set(NLOHMANN_JSON_INCLUDE_DIR "" CACHE PATH "nlohmann/json include directory")
if(NOT nlohmann_json_FOUND AND APPLE AND EXISTS "/opt/homebrew/opt/nlohmann-json/include/nlohmann/json.hpp")
    set(NLOHMANN_JSON_INCLUDE_DIR "/opt/homebrew/opt/nlohmann-json/include" CACHE PATH "nlohmann/json include directory" FORCE)
endif()
if(NOT nlohmann_json_FOUND AND (NOT NLOHMANN_JSON_INCLUDE_DIR OR NOT EXISTS "${NLOHMANN_JSON_INCLUDE_DIR}/nlohmann/json.hpp"))
    find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
        HINTS
            "/opt/homebrew/opt/nlohmann-json/include"
            "/usr/local/opt/nlohmann-json/include"
    )
endif()

# 1. oatpp
find_package(oatpp REQUIRED)

set(OATPP_INCLUDE_DIR "${OATPP_INCLUDE_DIR}" CACHE PATH "oatpp include directory")
if(NOT OATPP_INCLUDE_DIR OR NOT EXISTS "${OATPP_INCLUDE_DIR}/oatpp/core/Types.hpp")
    unset(OATPP_INCLUDE_DIR CACHE)
    find_path(OATPP_INCLUDE_DIR oatpp/core/Types.hpp
        HINTS
            "/opt/homebrew/opt/oatpp/include/oatpp-1.3.1/oatpp"
            "/opt/homebrew/opt/oatpp/include/oatpp-1.4.0/oatpp"
            "/usr/local/opt/oatpp/include/oatpp-1.3.1/oatpp"
            "/usr/local/opt/oatpp/include/oatpp-1.4.0/oatpp"
    )
endif()

if(NOT EXISTS "${OATPP_INCLUDE_DIR}/oatpp/core/Types.hpp")
    if(DEFINED oatpp_DIR)
        get_filename_component(OATPP_INSTALL_PREFIX "${oatpp_DIR}/../../.." ABSOLUTE)
        find_path(OATPP_INCLUDE_DIR oatpp/core/Types.hpp
            PATHS
                "${OATPP_INSTALL_PREFIX}/include/oatpp-1.3.0/oatpp"
                "${OATPP_INSTALL_PREFIX}/include/oatpp-1.3.1/oatpp"
                "${OATPP_INSTALL_PREFIX}/include/oatpp-1.4.0/oatpp"
        )
    endif()
endif()

if(NOT EXISTS "${OATPP_INCLUDE_DIR}/oatpp/core/Types.hpp")
    message(FATAL_ERROR "oatpp headers not found! Set OATPP_INCLUDE_DIR or oatpp_DIR.")
endif()

message(STATUS "Using oatpp include: ${OATPP_INCLUDE_DIR}")

# 2. OpenSSL
set(OPENSSL_ROOT_DIR "${OPENSSL_ROOT_DIR}" CACHE PATH "OpenSSL root directory")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_INCLUDE_DIR}" CACHE PATH "OpenSSL include directory")
set(OPENSSL_SSL_LIBRARY "${OPENSSL_SSL_LIBRARY}" CACHE FILEPATH "OpenSSL ssl library")
set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_CRYPTO_LIBRARY}" CACHE FILEPATH "OpenSSL crypto library")

if(APPLE AND EXISTS "${OPENSSL_ROOT_DIR}/include/openssl/ssl.h")
    if(OPENSSL_INCLUDE_DIR STREQUAL "")
        set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include" CACHE PATH "OpenSSL include directory" FORCE)
    endif()
    if(OPENSSL_SSL_LIBRARY STREQUAL "" AND EXISTS "${OPENSSL_ROOT_DIR}/lib/libssl.dylib")
        set(OPENSSL_SSL_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libssl.dylib" CACHE FILEPATH "OpenSSL ssl library" FORCE)
    endif()
    if(OPENSSL_CRYPTO_LIBRARY STREQUAL "" AND EXISTS "${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib")
        set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib" CACHE FILEPATH "OpenSSL crypto library" FORCE)
    endif()
endif()

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
set(MYSQL_ROOT_DIR "${MYSQL_ROOT_DIR}" CACHE PATH "MySQL Connector/C++ root directory")
set(MYSQL_INCLUDE_DIR "${MYSQL_INCLUDE_DIR}" CACHE PATH "MySQL Connector/C++ include directory")
set(MYSQL_LIB "${MYSQL_LIB}" CACHE FILEPATH "MySQL Connector/C++ library")
set(MYSQL_AUTO_FETCH_CLASSIC ON CACHE BOOL "Auto fetch MySQL Connector/C++ classic package on macOS if missing")
set(MYSQL_CONNECTOR_CPP_URL "https://cdn.mysql.com/Downloads/Connector-C++/mysql-connector-c++-9.7.0-macos15-arm64.tar.gz" CACHE STRING "MySQL Connector/C++ binary package URL")
set(MYSQL_CONNECTOR_CPP_CACHE_DIR "${CMAKE_BINARY_DIR}/_deps/mysql-connector-cpp" CACHE PATH "Cached MySQL Connector/C++ extraction directory")

if(NOT EXISTS "${MYSQL_INCLUDE_DIR}/mysql_driver.h")
    unset(MYSQL_INCLUDE_DIR CACHE)
    find_path(MYSQL_INCLUDE_DIR mysql_driver.h
        PATHS
            "${MYSQL_ROOT_DIR}"
            "${MYSQL_ROOT_DIR}/include"
            "${MYSQL_ROOT_DIR}/include/jdbc"
            "/opt/homebrew/opt/mysql-connector-c++/include/jdbc"
            "/usr/local/opt/mysql-connector-c++/include/jdbc"
            "/usr/include/mysql-cppconn-8/jdbc"
            "C:/Program Files/MySQL/MySQL Connector C++ 9.7/include/jdbc"
            "C:/Program Files/MySQL/Connector C++ 8.0/include/jdbc"
            "C:/Program Files/MySQL/MySQL Connector C++ 8.0/include/jdbc"
        PATH_SUFFIXES
            include/jdbc
            jdbc
    )
endif()

if(NOT EXISTS "${MYSQL_LIB}")
    unset(MYSQL_LIB CACHE)
    find_library(MYSQL_LIB mysqlcppconn
        PATHS
            "${MYSQL_ROOT_DIR}"
            "${MYSQL_ROOT_DIR}/lib"
            "${MYSQL_ROOT_DIR}/lib64"
            "/opt/homebrew/opt/mysql-connector-c++/lib"
            "/usr/local/opt/mysql-connector-c++/lib"
            "C:/Program Files/MySQL/MySQL Connector C++ 9.7/lib64/vs14"
            "C:/Program Files/MySQL/Connector C++ 8.0/lib64/vs14"
            "C:/Program Files/MySQL/MySQL Connector C++ 8.0/lib64/vs14"
        PATH_SUFFIXES
            lib
            lib64
            lib64/vs14
    )
endif()

if(APPLE AND MYSQL_AUTO_FETCH_CLASSIC AND
   (NOT EXISTS "${MYSQL_INCLUDE_DIR}/mysql_driver.h" OR NOT EXISTS "${MYSQL_LIB}"))
    message(STATUS "MySQL classic connector not found locally, trying auto-fetch package...")
    file(MAKE_DIRECTORY "${MYSQL_CONNECTOR_CPP_CACHE_DIR}")
    set(_mysql_tar "${MYSQL_CONNECTOR_CPP_CACHE_DIR}/mysql-connector-cpp.tar.gz")

    if(NOT EXISTS "${_mysql_tar}")
        file(DOWNLOAD
            "${MYSQL_CONNECTOR_CPP_URL}"
            "${_mysql_tar}"
            SHOW_PROGRESS
            STATUS _mysql_download_status
            TIMEOUT 120
        )
        list(GET _mysql_download_status 0 _mysql_download_code)
        if(NOT _mysql_download_code EQUAL 0)
            list(GET _mysql_download_status 1 _mysql_download_message)
            message(WARNING "MySQL Connector/C++ download failed: ${_mysql_download_message}")
        endif()
    endif()

    if(EXISTS "${_mysql_tar}")
        set(_mysql_unpack_dir "${MYSQL_CONNECTOR_CPP_CACHE_DIR}/unpacked")
        if(NOT EXISTS "${_mysql_unpack_dir}")
            file(MAKE_DIRECTORY "${_mysql_unpack_dir}")
            execute_process(
                COMMAND "${CMAKE_COMMAND}" -E tar xzf "${_mysql_tar}"
                WORKING_DIRECTORY "${_mysql_unpack_dir}"
                RESULT_VARIABLE _mysql_unpack_code
            )
            if(NOT _mysql_unpack_code EQUAL 0)
                message(WARNING "Failed to unpack MySQL Connector/C++ package.")
            endif()
        endif()

        file(GLOB _mysql_candidates LIST_DIRECTORIES true "${_mysql_unpack_dir}/mysql-connector-c++-*")
        if(_mysql_candidates)
            list(GET _mysql_candidates 0 _mysql_root_candidate)
            if(NOT MYSQL_ROOT_DIR)
                set(MYSQL_ROOT_DIR "${_mysql_root_candidate}" CACHE PATH "MySQL Connector/C++ root directory" FORCE)
            endif()
            if(NOT EXISTS "${MYSQL_INCLUDE_DIR}/mysql_driver.h")
                find_path(MYSQL_INCLUDE_DIR mysql_driver.h
                    PATHS
                        "${_mysql_root_candidate}/include/jdbc"
                        "${_mysql_root_candidate}/include"
                    PATH_SUFFIXES
                        jdbc
                        include/jdbc
                )
            endif()
            if(NOT EXISTS "${MYSQL_LIB}")
                find_library(MYSQL_LIB mysqlcppconn
                    PATHS
                        "${_mysql_root_candidate}/lib64"
                        "${_mysql_root_candidate}/lib"
                    PATH_SUFFIXES
                        lib64
                        lib
                )
            endif()
        endif()
    endif()
endif()

if(NOT EXISTS "${MYSQL_INCLUDE_DIR}/mysql_driver.h"
   OR NOT EXISTS "${MYSQL_INCLUDE_DIR}/cppconn/connection.h"
   OR NOT EXISTS "${MYSQL_LIB}")
    message(FATAL_ERROR
        "MySQL Connector/C++ (classic) not found.\n"
        "This project requires mysql_driver.h + libmysqlcppconn.\n"
        "Set MYSQL_INCLUDE_DIR and MYSQL_LIB manually, or install Connector/C++ 8.x classic JDBC.\n"
        "Note: Homebrew mysql-connector-c++ 9.x usually provides mysqlx/libmysqlcppconnx only, which is not compatible with current source code."
    )
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
    ${NLOHMANN_JSON_INCLUDE_DIR}
)

# 6. 链接库
target_link_libraries(jiewu_server PRIVATE
    oatpp::oatpp
    OpenSSL::SSL
    OpenSSL::Crypto
    "${MYSQLCPPCONN_LIBRARY}"
    # crypt32
    # ws2_32
    # bcrypt
    # advapi32
    # user32
)

if(nlohmann_json_FOUND)
    target_link_libraries(jiewu_server PRIVATE nlohmann_json::nlohmann_json)
elseif(NOT EXISTS "${NLOHMANN_JSON_INCLUDE_DIR}/nlohmann/json.hpp")
    message(FATAL_ERROR "nlohmann/json.hpp not found. Install nlohmann-json or set NLOHMANN_JSON_INCLUDE_DIR.")
endif()

if(APPLE)
    get_filename_component(MYSQLCPPCONN_LIB_DIR "${MYSQLCPPCONN_LIBRARY}" DIRECTORY)
    set(_RPATHS "${MYSQLCPPCONN_LIB_DIR}")
    if(EXISTS "${OPENSSL_ROOT_DIR}/lib")
        list(APPEND _RPATHS "${OPENSSL_ROOT_DIR}/lib")
    endif()
    set_target_properties(jiewu_server PROPERTIES
        BUILD_RPATH "${_RPATHS}"
        INSTALL_RPATH "${_RPATHS}"
    )

    # mysql classic dylib may depend on libssl/libcrypto by soname only.
    # Copy OpenSSL dylibs beside executable as runtime fallback on macOS.
    if(EXISTS "${OPENSSL_ROOT_DIR}/lib/libssl.3.dylib" AND EXISTS "${OPENSSL_ROOT_DIR}/lib/libcrypto.3.dylib")
        add_custom_command(TARGET jiewu_server POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${OPENSSL_ROOT_DIR}/lib/libssl.3.dylib"
                "$<TARGET_FILE_DIR:jiewu_server>/libssl.3.dylib"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${OPENSSL_ROOT_DIR}/lib/libcrypto.3.dylib"
                "$<TARGET_FILE_DIR:jiewu_server>/libcrypto.3.dylib"
        )
    endif()
endif()