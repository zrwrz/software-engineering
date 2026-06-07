#ifndef USER_REPOSITORY_HPP
#define USER_REPOSITORY_HPP

#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <string>
#include <optional>

/**
 * 实现各种数据库操作接口
 */
class UserRepository {
  public:
    /*
    仅供参考
    */
    static sql::Connection* getConnection();
    static bool existsByUsername(const std::string& username);
    static bool existsByPhone(const std::string& phone);
    static bool existsByEmail(const std::string& email);
    static int64_t createUser(
        const std::string& username,
        const std::string& phone,
        const std::string& email
    );

};

#endif