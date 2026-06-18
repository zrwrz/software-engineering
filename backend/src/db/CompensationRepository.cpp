#include "db/CompensationRepository.hpp"
#include "db/Database.hpp"

#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <algorithm>
#include <stdexcept>

namespace {
int normalizePage(int32_t page) { return page <= 0 ? 1 : page; }
int normalizeSize(int32_t size) {
    if (size <= 0) return 20;
    return std::min(size, 100);
}
}

oatpp::Object<CompensationRecordListResponse> CompensationRepository::listRecords(int32_t page, int32_t size, const std::string& status, int64_t userId, int64_t orderId) {
    page = normalizePage(page);
    size = normalizeSize(size);
    int offset = (page - 1) * size;
    auto conn = Database::getConnection();
    std::string where = " WHERE 1=1 ";
    if (!status.empty()) where += " AND status = ? ";
    if (userId > 0) where += " AND user_id = ? ";
    if (orderId > 0) where += " AND order_id = ? ";

    std::unique_ptr<sql::PreparedStatement> countStmt(conn->prepareStatement("SELECT COUNT(*) total FROM compensation_records" + where));
    int index = 1;
    if (!status.empty()) countStmt->setString(index++, status);
    if (userId > 0) countStmt->setInt64(index++, userId);
    if (orderId > 0) countStmt->setInt64(index++, orderId);
    std::unique_ptr<sql::ResultSet> countRs(countStmt->executeQuery());
    int64_t total = 0;
    if (countRs->next()) total = countRs->getInt64("total");

    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "SELECT id, order_id, user_id, amount, reason, status, DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') created_at "
        "FROM compensation_records" + where + " ORDER BY created_at DESC LIMIT ? OFFSET ?"
    ));
    index = 1;
    if (!status.empty()) stmt->setString(index++, status);
    if (userId > 0) stmt->setInt64(index++, userId);
    if (orderId > 0) stmt->setInt64(index++, orderId);
    stmt->setInt(index++, size);
    stmt->setInt(index++, offset);

    auto response = CompensationRecordListResponse::createShared();
    response->list = oatpp::Vector<oatpp::Object<CompensationRecordDto>>::createShared();
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    while (rs->next()) {
        auto record = CompensationRecordDto::createShared();
        record->id = rs->getInt64("id");
        record->orderId = rs->getInt64("order_id");
        record->userId = rs->getInt64("user_id");
        record->amount = rs->getDouble("amount");
        record->reason = rs->getString("reason").c_str();
        record->status = rs->getString("status").c_str();
        record->createdAt = rs->getString("created_at").c_str();
        response->list->push_back(record);
    }
    response->page = page;
    response->size = size;
    response->total = total;
    return response;
}

oatpp::Object<CompensationRecordDto> CompensationRepository::createRecord(const oatpp::Object<CompensationCreateRequest>& request) {
    auto conn = Database::getConnection();
    conn->setAutoCommit(false);
    try {
        std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
            "INSERT INTO compensation_records(order_id, user_id, amount, reason, status) VALUES(?, ?, ?, ?, 'PENDING')"
        ));
        stmt->setInt64(1, request->orderId.getValue(0));
        stmt->setInt64(2, request->userId.getValue(0));
        stmt->setDouble(3, request->amount.getValue(0.0));
        stmt->setString(4, request->reason ? request->reason->c_str() : "");
        stmt->executeUpdate();

        std::unique_ptr<sql::Statement> idStmt(conn->createStatement());
        std::unique_ptr<sql::ResultSet> idRs(idStmt->executeQuery("SELECT LAST_INSERT_ID() id"));
        int64_t id = 0;
        if (idRs->next()) id = idRs->getInt64("id");

        std::unique_ptr<sql::PreparedStatement> creditStmt(conn->prepareStatement(
            "INSERT INTO credit_records(user_id, order_id, change_value, reason) VALUES(?, ?, -10, '损坏赔偿')"
        ));
        creditStmt->setInt64(1, request->userId.getValue(0));
        creditStmt->setInt64(2, request->orderId.getValue(0));
        creditStmt->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> userStmt(conn->prepareStatement(
            "UPDATE users SET credit_score = credit_score - 10 WHERE id = ?"
        ));
        userStmt->setInt64(1, request->userId.getValue(0));
        userStmt->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> noticeStmt(conn->prepareStatement(
            "INSERT INTO notifications(user_id, type, title, content, related_order_id) VALUES(?, 'COMPENSATION', '赔偿处理通知', '您有新的赔偿记录待处理。', ?)"
        ));
        noticeStmt->setInt64(1, request->userId.getValue(0));
        noticeStmt->setInt64(2, request->orderId.getValue(0));
        noticeStmt->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> query(conn->prepareStatement(
            "SELECT id, order_id, user_id, amount, reason, status, DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') created_at FROM compensation_records WHERE id = ?"
        ));
        query->setInt64(1, id);
        std::unique_ptr<sql::ResultSet> rs(query->executeQuery());
        if (!rs->next()) throw std::runtime_error("赔偿记录创建失败");

        conn->commit();
        auto record = CompensationRecordDto::createShared();
        record->id = rs->getInt64("id");
        record->orderId = rs->getInt64("order_id");
        record->userId = rs->getInt64("user_id");
        record->amount = rs->getDouble("amount");
        record->reason = rs->getString("reason").c_str();
        record->status = rs->getString("status").c_str();
        record->createdAt = rs->getString("created_at").c_str();
        return record;
    } catch (...) {
        conn->rollback();
        throw;
    }
}

oatpp::Object<StatusResultDto> CompensationRepository::updateStatus(int64_t id, const std::string& status) {
    auto conn = Database::getConnection();
    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement("UPDATE compensation_records SET status = ? WHERE id = ?"));
    stmt->setString(1, status);
    stmt->setInt64(2, id);
    int affected = stmt->executeUpdate();
    if (affected == 0) throw std::runtime_error("赔偿记录不存在");

    auto result = StatusResultDto::createShared();
    result->id = id;
    result->status = status.c_str();
    return result;
}
