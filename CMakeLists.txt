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

# JWT 工具使用 OpenSSL HMAC
find_package(OpenSSL REQUIRED)

# 管理端与辅助功能的数据访问层使用 MySQL Connector/C++
# 如未安装 mysqlcppconn，仅编译部分 main.cpp。
find_library(MYSQLCPPCONN_LIBRARY mysqlcppconn)

set(PROJECT_SOURCES
    src/main.cpp
    # 其他 .cpp 文件
)

if(MYSQLCPPCONN_LIBRARY)
    list(APPEND PROJECT_SOURCES
        src/db/Database.cpp
        src/db/AuditLogRepository.cpp
        src/db/AdminUserRepository.cpp
        src/db/ItemManageRepository.cpp
        src/db/OrderManageRepository.cpp
        src/db/CreditRepository.cpp
        src/db/CompensationRepository.cpp
        src/db/StatisticsRepository.cpp
        src/service/AdminUserService.cpp
        src/service/ItemManageService.cpp
        src/service/OrderManageService.cpp
        src/service/CreditService.cpp
        src/service/CompensationService.cpp
        src/service/StatisticsService.cpp
    )
else()
    message(WARNING "未找到 mysqlcppconn")
endif()

# 添加可执行文件
add_executable(jiewu_server
    ${PROJECT_SOURCES}
add_executable(jiewu_server
    src/main.cpp
    src/db/UserRepository.cpp
    src/service/UserService.cpp
)

target_link_libraries(jiewu_server
    oatpp::oatpp
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(MYSQLCPPCONN_LIBRARY)
    target_link_libraries(jiewu_server
        ${MYSQLCPPCONN_LIBRARY}
    )
endif()

# 设置头文件搜索路径
    OpenSSL::Crypto
    ${MYSQLCPPCONN_LIBRARY}
)

target_include_directories(jiewu_server PRIVATE
    src
    ${MYSQLCPPCONN_INCLUDE_DIR}
    /usr/local/include/oatpp-1.4.0/oatpp
)
