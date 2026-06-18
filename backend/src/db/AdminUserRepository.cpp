#include "db/AdminUserRepository.hpp"
#include "db/Database.hpp"

#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <algorithm>
#include <stdexcept>

namespace {
int normalizePage(int32_t page) { return page <= 0 ? 1 : page; }
int normalizeSize(int32_t size) {
    if (size <= 0) return 10;
    return std::min(size, 100);
}
}

oatpp::Object<AdminUserListResponse> AdminUserRepository::listUsers(int32_t page, int32_t size, const std::string& keyword, const std::string& status) {
    page = normalizePage(page);
    size = normalizeSize(size);
    int offset = (page - 1) * size;

    auto conn = Database::getConnection();
    std::string where = " WHERE 1=1 ";
    if (!keyword.empty()) where += " AND (username LIKE ? OR phone LIKE ? OR email LIKE ?) ";
    if (!status.empty()) where += " AND status = ? ";

    std::unique_ptr<sql::PreparedStatement> countStmt(conn->prepareStatement("SELECT COUNT(*) total FROM users" + where));
    int index = 1;
    if (!keyword.empty()) {
        std::string likeValue = "%" + keyword + "%";
        countStmt->setString(index++, likeValue);
        countStmt->setString(index++, likeValue);
        countStmt->setString(index++, likeValue);
    }
    if (!status.empty()) countStmt->setString(index++, status);
    std::unique_ptr<sql::ResultSet> countRs(countStmt->executeQuery());
    int64_t total = 0;
    if (countRs->next()) total = countRs->getInt64("total");

    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "SELECT id, username, role, phone, credit_score, status, DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') created_at "
        "FROM users" + where + " ORDER BY created_at DESC LIMIT ? OFFSET ?"
    ));
    index = 1;
    if (!keyword.empty()) {
        std::string likeValue = "%" + keyword + "%";
        stmt->setString(index++, likeValue);
        stmt->setString(index++, likeValue);
        stmt->setString(index++, likeValue);
    }
    if (!status.empty()) stmt->setString(index++, status);
    stmt->setInt(index++, size);
    stmt->setInt(index++, offset);

    auto response = AdminUserListResponse::createShared();
    response->list = oatpp::Vector<oatpp::Object<AdminUserListItemDto>>::createShared();
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    while (rs->next()) {
        auto user = AdminUserListItemDto::createShared();
        user->id = rs->getInt64("id");
        user->username = rs->getString("username").c_str();
        user->role = rs->getString("role").c_str();
        user->phone = rs->getString("phone").c_str();
        user->creditScore = rs->getInt("credit_score");
        user->status = rs->getString("status").c_str();
        user->createdAt = rs->getString("created_at").c_str();
        response->list->push_back(user);
    }
    response->page = page;
    response->size = size;
    response->total = total;
    return response;
}

oatpp::Object<StatusResultDto> AdminUserRepository::updateUserStatus(int64_t userId, const std::string& status) {
    auto conn = Database::getConnection();
    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "UPDATE users SET status = ? WHERE id = ?"
    ));
    stmt->setString(1, status);
    stmt->setInt64(2, userId);
    int affected = stmt->executeUpdate();
    if (affected == 0) {
        throw std::runtime_error("用户不存在");
    }

    auto result = StatusResultDto::createShared();
    result->id = userId;
    result->status = status.c_str();
    return result;
}
