#ifndef USER_REPOSITORY_HPP
#define USER_REPOSITORY_HPP

#include <cppconn/connection.h>
#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cstdint>
#include <optional>
#include <string>

class UserRepository {
  public:
    struct UserRecord {
        int64_t id;
        std::string username;
        std::string password;
        std::string role;
        std::string phone;
        std::string email;
        int32_t creditScore;
        std::string status;
        std::string createdAt;
    };

    static sql::Connection* getConnection();

    static bool existsByUsername(const std::string& username);
    static bool existsByPhone(const std::string& phone);
    static bool existsByEmail(const std::string& email);
    static bool existsByPhoneForOtherUser(const std::string& phone, int64_t userId);
    static bool existsByEmailForOtherUser(const std::string& email, int64_t userId);

    static std::optional<UserRecord> findByUsername(const std::string& username);
    static std::optional<UserRecord> findById(int64_t userId);

    static int64_t createUser(
        const std::string& username,
        const std::string& passwordHash,
        const std::string& phone,
        const std::string& email
    );

    static bool updateProfile(
        int64_t userId,
        const std::string& phone,
        const std::string& email
    );

    static bool updatePassword(int64_t userId, const std::string& passwordHash);
};

#endif
