#include "db/UserRepository.hpp"

#include <cppconn/datatype.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <mysql_driver.h>

#include <cstdlib>
#include <functional>
#include <memory>
#include <stdexcept>

namespace {

std::string getEnvOrDefault(const char* name, const std::string& defaultValue) {
    const char* value = std::getenv(name);
    if (value == nullptr || value[0] == '\0') {
        return defaultValue;
    }
    return value;
}

std::runtime_error dbError(const std::string& action, const sql::SQLException& e) {
    return std::runtime_error(action + " failed: " + e.what());
}

UserRepository::UserRecord mapUser(sql::ResultSet* result) {
    UserRepository::UserRecord user;
    user.id = result->getInt64("id");
    user.username = result->getString("username");
    user.password = result->getString("password");
    user.role = result->getString("role");
    user.phone = result->getString("phone");
    user.email = result->getString("email");
    user.creditScore = result->getInt("credit_score");
    user.status = result->getString("status");
    user.createdAt = result->getString("created_at");
    return user;
}

bool existsByColumn(const std::string& column, const std::string& value) {
    if (value.empty()) {
        return false;
    }

    try {
        std::unique_ptr<sql::Connection> conn(UserRepository::getConnection());
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("SELECT 1 FROM users WHERE " + column + " = ? LIMIT 1")
        );
        stmt->setString(1, value);

        std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
        return result->next();
    } catch (const sql::SQLException& e) {
        throw dbError("check user existence", e);
    }
}

bool existsByColumnForOtherUser(
    const std::string& column,
    const std::string& value,
    int64_t userId
) {
    if (value.empty()) {
        return false;
    }

    try {
        std::unique_ptr<sql::Connection> conn(UserRepository::getConnection());
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("SELECT 1 FROM users WHERE " + column + " = ? AND id <> ? LIMIT 1")
        );
        stmt->setString(1, value);
        stmt->setInt64(2, userId);

        std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
        return result->next();
    } catch (const sql::SQLException& e) {
        throw dbError("check user uniqueness", e);
    }
}

std::optional<UserRepository::UserRecord> findOne(
    const std::string& whereClause,
    const std::function<void(sql::PreparedStatement*)>& bindParams
) {
    try {
        std::unique_ptr<sql::Connection> conn(UserRepository::getConnection());
        std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
            "SELECT id, username, password, role, phone, COALESCE(email, '') AS email, "
            "credit_score, status, DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') AS created_at "
            "FROM users WHERE " + whereClause + " LIMIT 1"
        ));
        bindParams(stmt.get());

        std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
        if (!result->next()) {
            return std::nullopt;
        }

        return mapUser(result.get());
    } catch (const sql::SQLException& e) {
        throw dbError("find user", e);
    }
}

} // namespace

sql::Connection* UserRepository::getConnection() {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();

        const std::string host = getEnvOrDefault("JIEWU_DB_HOST", "DB_HOST", "tcp://127.0.0.1:3306");
        const std::string user = getEnvOrDefault("JIEWU_DB_USER", "DB_USER", "root");
        const std::string password = getEnvOrDefault("JIEWU_DB_PASSWORD", "DB_PASSWORD", "password");
        const std::string database = getEnvOrDefault("JIEWU_DB_NAME", "DB_NAME", "jiewu_project");

        sql::Connection* conn = driver->connect(host, user, password);
        conn->setSchema(database);
        return conn;
    } catch (const sql::SQLException& e) {
        throw dbError("connect database", e);
    }
}

bool UserRepository::existsByUsername(const std::string& username) {
    return existsByColumn("username", username);
}

bool UserRepository::existsByPhone(const std::string& phone) {
    return existsByColumn("phone", phone);
}

bool UserRepository::existsByEmail(const std::string& email) {
    return existsByColumn("email", email);
}

bool UserRepository::existsByPhoneForOtherUser(const std::string& phone, int64_t userId) {
    return existsByColumnForOtherUser("phone", phone, userId);
}

bool UserRepository::existsByEmailForOtherUser(const std::string& email, int64_t userId) {
    return existsByColumnForOtherUser("email", email, userId);
}

std::optional<UserRepository::UserRecord> UserRepository::findByUsername(
    const std::string& username
) {
    return findOne("username = ?", [&](sql::PreparedStatement* stmt) {
        stmt->setString(1, username);
    });
}

std::optional<UserRepository::UserRecord> UserRepository::findById(int64_t userId) {
    return findOne("id = ?", [&](sql::PreparedStatement* stmt) {
        stmt->setInt64(1, userId);
    });
}

int64_t UserRepository::createUser(
    const std::string& username,
    const std::string& passwordHash,
    const std::string& phone,
    const std::string& email
) {
    try {
        std::unique_ptr<sql::Connection> conn(getConnection());
        std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
            "INSERT INTO users (username, password, phone, email) VALUES (?, ?, ?, ?)"
        ));
        stmt->setString(1, username);
        stmt->setString(2, passwordHash);
        stmt->setString(3, phone);
        if (email.empty()) {
            stmt->setNull(4, sql::DataType::VARCHAR);
        } else {
            stmt->setString(4, email);
        }

        stmt->executeUpdate();

        std::unique_ptr<sql::Statement> idStmt(conn->createStatement());
        std::unique_ptr<sql::ResultSet> result(idStmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
        if (!result->next()) {
            throw std::runtime_error("create user failed: missing generated id");
        }

        return result->getInt64("id");
    } catch (const sql::SQLException& e) {
        throw dbError("create user", e);
    }
}

bool UserRepository::updateProfile(
    int64_t userId,
    const std::string& phone,
    const std::string& email
) {
    try {
        std::unique_ptr<sql::Connection> conn(getConnection());
        std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
            "UPDATE users SET phone = ?, email = ? WHERE id = ?"
        ));
        stmt->setString(1, phone);
        if (email.empty()) {
            stmt->setNull(2, sql::DataType::VARCHAR);
        } else {
            stmt->setString(2, email);
        }
        stmt->setInt64(3, userId);

        return stmt->executeUpdate() > 0;
    } catch (const sql::SQLException& e) {
        throw dbError("update user profile", e);
    }
}

bool UserRepository::updatePassword(int64_t userId, const std::string& passwordHash) {
    try {
        std::unique_ptr<sql::Connection> conn(getConnection());
        std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
            "UPDATE users SET password = ? WHERE id = ?"
        ));
        stmt->setString(1, passwordHash);
        stmt->setInt64(2, userId);

        return stmt->executeUpdate() > 0;
    } catch (const sql::SQLException& e) {
        throw dbError("update user password", e);
    }
}
