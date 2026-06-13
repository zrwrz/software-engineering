#include "db/Database.hpp"

#include <cppconn/driver.h>
#include <mysql_driver.h>
#include <cstdlib>
#include <stdexcept>

std::string Database::getEnvOrDefault(const char* key, const std::string& defaultValue) {
    const char* value = std::getenv(key);
    if (value == nullptr || std::string(value).empty()) {
        return defaultValue;
    }
    return std::string(value);
}

std::string getCompatibleEnv(const char* primaryKey, const char* fallbackKey, const std::string& defaultValue) {
    const char* value = std::getenv(primaryKey);
    if (value != nullptr && !std::string(value).empty()) {
        return std::string(value);
    }
    value = std::getenv(fallbackKey);
    if (value != nullptr && !std::string(value).empty()) {
        return std::string(value);
    }
    return defaultValue;
}

std::unique_ptr<sql::Connection> Database::getConnection() {
    sql::Driver* driver = get_driver_instance();
    if (driver == nullptr) {
        throw std::runtime_error("数据库驱动初始化失败");
    }

    std::string host = getCompatibleEnv("JIEWU_DB_HOST", "DB_HOST", "tcp://127.0.0.1:3306");
    std::string user = getCompatibleEnv("JIEWU_DB_USER", "DB_USER", "root");
    std::string password = getCompatibleEnv("JIEWU_DB_PASSWORD", "DB_PASSWORD", "password");
    std::string database = getCompatibleEnv("JIEWU_DB_NAME", "DB_NAME", "jiewu_project");

    auto connection = std::unique_ptr<sql::Connection>(driver->connect(host, user, password));
    connection->setSchema(database);
    return connection;
}
