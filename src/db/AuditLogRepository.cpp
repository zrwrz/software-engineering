#include "db/AuditLogRepository.hpp"
#include "db/Database.hpp"

#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <sstream>

namespace {
int normalizePage(int32_t page) { return page <= 0 ? 1 : page; }
int normalizeSize(int32_t size) { return size <= 0 ? 10 : size; }
}

void AuditLogRepository::createLog(int64_t operatorId, const std::string& action, const std::string& target, const std::string& detail) {
    auto conn = Database::getConnection();
    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "INSERT INTO audit_logs(operator_id, action, target, detail) VALUES(?, ?, ?, ?)"
    ));
    stmt->setInt64(1, operatorId);
    stmt->setString(2, action);
    stmt->setString(3, target);
    stmt->setString(4, detail);
    stmt->executeUpdate();
}

oatpp::Object<AuditLogListResponse> AuditLogRepository::listLogs(int32_t page, int32_t size, int64_t operatorId, const std::string& startTime, const std::string& endTime) {
    page = normalizePage(page);
    size = normalizeSize(size);
    int offset = (page - 1) * size;

    auto conn = Database::getConnection();
    std::string where = " WHERE 1=1 ";
    if (operatorId > 0) where += " AND operator_id = ? ";
    if (!startTime.empty()) where += " AND created_at >= ? ";
    if (!endTime.empty()) where += " AND created_at <= ? ";

    std::unique_ptr<sql::PreparedStatement> countStmt(conn->prepareStatement("SELECT COUNT(*) total FROM audit_logs" + where));
    int index = 1;
    if (operatorId > 0) countStmt->setInt64(index++, operatorId);
    if (!startTime.empty()) countStmt->setString(index++, startTime);
    if (!endTime.empty()) countStmt->setString(index++, endTime);
    std::unique_ptr<sql::ResultSet> countRs(countStmt->executeQuery());
    int64_t total = 0;
    if (countRs->next()) total = countRs->getInt64("total");

    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "SELECT id, operator_id, action, target, detail, DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') created_at "
        "FROM audit_logs" + where + " ORDER BY created_at DESC LIMIT ? OFFSET ?"
    ));
    index = 1;
    if (operatorId > 0) stmt->setInt64(index++, operatorId);
    if (!startTime.empty()) stmt->setString(index++, startTime);
    if (!endTime.empty()) stmt->setString(index++, endTime);
    stmt->setInt(index++, size);
    stmt->setInt(index++, offset);

    auto response = AuditLogListResponse::createShared();
    response->list = oatpp::Vector<oatpp::Object<AuditLogDto>>::createShared();
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    while (rs->next()) {
        auto item = AuditLogDto::createShared();
        item->id = rs->getInt64("id");
        item->operatorId = rs->getInt64("operator_id");
        item->action = rs->getString("action").c_str();
        item->target = rs->getString("target").c_str();
        item->detail = rs->getString("detail").c_str();
        item->createdAt = rs->getString("created_at").c_str();
        response->list->push_back(item);
    }
    response->page = page;
    response->size = size;
    response->total = total;
    return response;
}
