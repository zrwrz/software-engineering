#include "db/CreditRepository.hpp"
#include "db/Database.hpp"

#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <algorithm>
#include <stdexcept>

namespace {
int normalizePage(int32_t page) { return page <= 0 ? 1 : page; }
int normalizeSize(int32_t size) {
    if (size <= 0) return 10;
    return std::min(size, 100);
}
}

oatpp::Object<CreditRecordListResponse> CreditRepository::listRecords(int64_t userId, int32_t page, int32_t size) {
    page = normalizePage(page);
    size = normalizeSize(size);
    int offset = (page - 1) * size;
    auto conn = Database::getConnection();
    std::string where = userId > 0 ? " WHERE user_id = ? " : " WHERE 1=1 ";

    std::unique_ptr<sql::PreparedStatement> countStmt(conn->prepareStatement("SELECT COUNT(*) total FROM credit_records" + where));
    if (userId > 0) countStmt->setInt64(1, userId);
    std::unique_ptr<sql::ResultSet> countRs(countStmt->executeQuery());
    int64_t total = 0;
    if (countRs->next()) total = countRs->getInt64("total");

    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "SELECT id, user_id, order_id, change_value, reason, DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') created_at "
        "FROM credit_records" + where + " ORDER BY created_at DESC LIMIT ? OFFSET ?"
    ));
    int index = 1;
    if (userId > 0) stmt->setInt64(index++, userId);
    stmt->setInt(index++, size);
    stmt->setInt(index++, offset);

    auto response = CreditRecordListResponse::createShared();
    response->list = oatpp::Vector<oatpp::Object<CreditRecordDto>>::createShared();
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    while (rs->next()) {
        auto record = CreditRecordDto::createShared();
        record->id = rs->getInt64("id");
        record->userId = rs->getInt64("user_id");
        record->orderId = rs->isNull("order_id") ? 0 : rs->getInt64("order_id");
        record->changeValue = rs->getInt("change_value");
        record->reason = rs->getString("reason").c_str();
        record->createdAt = rs->getString("created_at").c_str();
        response->list->push_back(record);
    }
    response->page = page;
    response->size = size;
    response->total = total;
    return response;
}

oatpp::Object<CreditAdjustResultDto> CreditRepository::adjustCredit(const oatpp::Object<CreditAdjustRequest>& request) {
    auto conn = Database::getConnection();
    conn->setAutoCommit(false);
    try {
        std::unique_ptr<sql::PreparedStatement> updateUser(conn->prepareStatement(
            "UPDATE users SET credit_score = credit_score + ? WHERE id = ?"
        ));
        updateUser->setInt(1, request->changeValue.getValue(0));
        updateUser->setInt64(2, request->userId.getValue(0));
        if (updateUser->executeUpdate() == 0) throw std::runtime_error("用户不存在");

        std::unique_ptr<sql::PreparedStatement> insertRecord(conn->prepareStatement(
            "INSERT INTO credit_records(user_id, order_id, change_value, reason) VALUES(?, NULL, ?, ?)"
        ));
        insertRecord->setInt64(1, request->userId.getValue(0));
        insertRecord->setInt(2, request->changeValue.getValue(0));
        insertRecord->setString(3, request->reason ? request->reason->c_str() : "手工调整");
        insertRecord->executeUpdate();

        std::unique_ptr<sql::Statement> idStmt(conn->createStatement());
        std::unique_ptr<sql::ResultSet> idRs(idStmt->executeQuery("SELECT LAST_INSERT_ID() id"));
        int64_t recordId = 0;
        if (idRs->next()) recordId = idRs->getInt64("id");

        std::unique_ptr<sql::PreparedStatement> queryUser(conn->prepareStatement("SELECT credit_score FROM users WHERE id = ?"));
        queryUser->setInt64(1, request->userId.getValue(0));
        std::unique_ptr<sql::ResultSet> userRs(queryUser->executeQuery());
        int currentScore = 0;
        if (userRs->next()) currentScore = userRs->getInt("credit_score");

        std::unique_ptr<sql::PreparedStatement> queryTime(conn->prepareStatement(
            "SELECT DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') created_at FROM credit_records WHERE id = ?"
        ));
        queryTime->setInt64(1, recordId);
        std::unique_ptr<sql::ResultSet> timeRs(queryTime->executeQuery());

        conn->commit();
        auto result = CreditAdjustResultDto::createShared();
        result->id = recordId;
        result->userId = request->userId;
        result->changeValue = request->changeValue;
        result->reason = request->reason;
        result->currentCreditScore = currentScore;
        if (timeRs->next()) result->createdAt = timeRs->getString("created_at").c_str();
        return result;
    } catch (...) {
        conn->rollback();
        throw;
    }
}
