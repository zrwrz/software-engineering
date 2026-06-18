#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <cppconn/connection.h>
#include <memory>
#include <string>

/**
 * 数据库连接工具
 */
class Database {
  public:
    /**
     * @brief 创建 MySQL 数据库连接
     * 
     * 连接参数可通过环境变量覆盖：
     * JIEWU_DB_HOST、JIEWU_DB_USER、JIEWU_DB_PASSWORD、JIEWU_DB_NAME。
     * 
     * @return std::unique_ptr<sql::Connection> 数据库连接
     */
    static std::unique_ptr<sql::Connection> getConnection();

  private:
    static std::string getEnvOrDefault(const char* key, const std::string& defaultValue);
};

#endif
