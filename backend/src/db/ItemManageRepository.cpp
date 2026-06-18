#include "db/ItemManageRepository.hpp"
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

oatpp::Object<ItemListResponse> ItemManageRepository::listItems(int32_t page, int32_t size, const std::string& keyword, const std::string& category, const std::string& status) {
    page = normalizePage(page);
    size = normalizeSize(size);
    int offset = (page - 1) * size;

    auto conn = Database::getConnection();
    std::string where = " WHERE 1=1 ";
    if (!keyword.empty()) where += " AND (name LIKE ? OR description LIKE ?) ";
    if (!category.empty()) where += " AND category = ? ";
    if (!status.empty()) where += " AND status = ? ";

    std::unique_ptr<sql::PreparedStatement> countStmt(conn->prepareStatement("SELECT COUNT(*) total FROM items" + where));
    int index = 1;
    if (!keyword.empty()) {
        std::string likeValue = "%" + keyword + "%";
        countStmt->setString(index++, likeValue);
        countStmt->setString(index++, likeValue);
    }
    if (!category.empty()) countStmt->setString(index++, category);
    if (!status.empty()) countStmt->setString(index++, status);
    std::unique_ptr<sql::ResultSet> countRs(countStmt->executeQuery());
    int64_t total = 0;
    if (countRs->next()) total = countRs->getInt64("total");

    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "SELECT id, name, category, available_count, status FROM items" + where + " ORDER BY created_at DESC LIMIT ? OFFSET ?"
    ));
    index = 1;
    if (!keyword.empty()) {
        std::string likeValue = "%" + keyword + "%";
        stmt->setString(index++, likeValue);
        stmt->setString(index++, likeValue);
    }
    if (!category.empty()) stmt->setString(index++, category);
    if (!status.empty()) stmt->setString(index++, status);
    stmt->setInt(index++, size);
    stmt->setInt(index++, offset);

    auto response = ItemListResponse::createShared();
    response->list = oatpp::Vector<oatpp::Object<ItemListItemDto>>::createShared();
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    while (rs->next()) {
        auto item = ItemListItemDto::createShared();
        item->id = rs->getInt64("id");
        item->name = rs->getString("name").c_str();
        item->category = rs->getString("category").c_str();
        item->availableCount = rs->getInt("available_count");
        item->status = rs->getString("status").c_str();
        response->list->push_back(item);
    }
    response->page = page;
    response->size = size;
    response->total = static_cast<v_int32>(total);
    return response;
}

oatpp::Object<ItemDetailedResponse> ItemManageRepository::getItemDetail(int64_t itemId) {
    auto conn = Database::getConnection();
    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "SELECT id, name, category, description, total_count, available_count, status, deposit, created_by, "
        "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') created_at FROM items WHERE id = ?"
    ));
    stmt->setInt64(1, itemId);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) throw std::runtime_error("物品不存在");

    auto item = ItemDetailedResponse::createShared();
    item->id = rs->getInt64("id");
    item->name = rs->getString("name").c_str();
    item->category = rs->getString("category").c_str();
    item->description = rs->getString("description").c_str();
    item->totalCount = rs->getInt("total_count");
    item->availableCount = rs->getInt("available_count");
    item->status = rs->getString("status").c_str();
    item->deposit = rs->getDouble("deposit");
    item->createdBy = rs->getInt64("created_by");
    item->createdAt = rs->getString("created_at").c_str();
    return item;
}

oatpp::Object<StatusResultDto> ItemManageRepository::createSubmission(int64_t submitterId, const oatpp::Object<ItemSubmissionRequest>& request) {
    auto conn = Database::getConnection();
    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "INSERT INTO item_submissions(submitter_id, name, category, description, total_count, deposit, status) VALUES(?, ?, ?, ?, ?, ?, 'PENDING')"
    ));
    stmt->setInt64(1, submitterId);
    stmt->setString(2, request->name ? request->name->c_str() : "");
    stmt->setString(3, request->category ? request->category->c_str() : "");
    stmt->setString(4, request->description ? request->description->c_str() : "");
    stmt->setInt(5, request->totalCount ? request->totalCount.getValue(0) : 0);
    stmt->setDouble(6, request->deposit ? request->deposit.getValue(0.0) : 0.0);
    stmt->executeUpdate();

    std::unique_ptr<sql::Statement> idStmt(conn->createStatement());
    std::unique_ptr<sql::ResultSet> rs(idStmt->executeQuery("SELECT LAST_INSERT_ID() id"));
    int64_t submissionId = 0;
    if (rs->next()) submissionId = rs->getInt64("id");

    auto result = StatusResultDto::createShared();
    result->id = submissionId;
    result->status = "PENDING";
    return result;
}

oatpp::Object<ItemSubmissionListResponse> ItemManageRepository::listSubmissions(int32_t page, int32_t size, const std::string& status) {
    page = normalizePage(page);
    size = normalizeSize(size);
    int offset = (page - 1) * size;
    auto conn = Database::getConnection();
    std::string where = " WHERE 1=1 ";
    if (!status.empty()) where += " AND status = ? ";

    std::unique_ptr<sql::PreparedStatement> countStmt(conn->prepareStatement("SELECT COUNT(*) total FROM item_submissions" + where));
    if (!status.empty()) countStmt->setString(1, status);
    std::unique_ptr<sql::ResultSet> countRs(countStmt->executeQuery());
    int64_t total = 0;
    if (countRs->next()) total = countRs->getInt64("total");

    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "SELECT id, submitter_id, name, category, description, total_count, deposit, audit_remark, status, "
        "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') created_at FROM item_submissions" + where + " ORDER BY created_at DESC LIMIT ? OFFSET ?"
    ));
    int index = 1;
    if (!status.empty()) stmt->setString(index++, status);
    stmt->setInt(index++, size);
    stmt->setInt(index++, offset);

    auto response = ItemSubmissionListResponse::createShared();
    response->list = oatpp::Vector<oatpp::Object<ItemSubmissionDto>>::createShared();
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    while (rs->next()) {
        auto dto = ItemSubmissionDto::createShared();
        dto->id = rs->getInt64("id");
        dto->submitterId = rs->getInt64("submitter_id");
        dto->name = rs->getString("name").c_str();
        dto->category = rs->getString("category").c_str();
        dto->description = rs->getString("description").c_str();
        dto->totalCount = rs->getInt("total_count");
        dto->deposit = rs->getDouble("deposit");
        dto->auditRemark = rs->isNull("audit_remark") ? nullptr : oatpp::String(rs->getString("audit_remark").c_str());
        dto->status = rs->getString("status").c_str();
        dto->createdAt = rs->getString("created_at").c_str();
        response->list->push_back(dto);
    }
    response->page = page;
    response->size = size;
    response->total = total;
    return response;
}

oatpp::Object<StatusResultDto> ItemManageRepository::auditSubmission(int64_t operatorId, int64_t submissionId, const std::string& action, const std::string& remark) {
    auto conn = Database::getConnection();
    conn->setAutoCommit(false);
    try {
        std::unique_ptr<sql::PreparedStatement> selectStmt(conn->prepareStatement(
            "SELECT id, submitter_id, name, category, description, total_count, deposit, status FROM item_submissions WHERE id = ? FOR UPDATE"
        ));
        selectStmt->setInt64(1, submissionId);
        std::unique_ptr<sql::ResultSet> rs(selectStmt->executeQuery());
        if (!rs->next()) throw std::runtime_error("物品上传申请不存在");
        if (rs->getString("status") != "PENDING") throw std::runtime_error("申请状态不允许当前操作");

        std::string newStatus = action == "APPROVE" ? "APPROVED" : "REJECTED";
        int64_t itemId = 0;
        if (action == "APPROVE") {
            std::unique_ptr<sql::PreparedStatement> insertItem(conn->prepareStatement(
                "INSERT INTO items(name, category, description, total_count, available_count, status, deposit, created_by) VALUES(?, ?, ?, ?, ?, 'ON_SHELF', ?, ?)"
            ));
            insertItem->setString(1, rs->getString("name"));
            insertItem->setString(2, rs->getString("category"));
            insertItem->setString(3, rs->getString("description"));
            insertItem->setInt(4, rs->getInt("total_count"));
            insertItem->setInt(5, rs->getInt("total_count"));
            insertItem->setDouble(6, rs->getDouble("deposit"));
            insertItem->setInt64(7, operatorId);
            insertItem->executeUpdate();
            std::unique_ptr<sql::Statement> idStmt(conn->createStatement());
            std::unique_ptr<sql::ResultSet> idRs(idStmt->executeQuery("SELECT LAST_INSERT_ID() id"));
            if (idRs->next()) itemId = idRs->getInt64("id");
        }

        std::unique_ptr<sql::PreparedStatement> updateStmt(conn->prepareStatement(
            "UPDATE item_submissions SET status = ?, audit_remark = ?, auditor_id = ?, audited_at = NOW() WHERE id = ?"
        ));
        updateStmt->setString(1, newStatus);
        updateStmt->setString(2, remark);
        updateStmt->setInt64(3, operatorId);
        updateStmt->setInt64(4, submissionId);
        updateStmt->executeUpdate();

        conn->commit();
        auto result = StatusResultDto::createShared();
        result->id = submissionId;
        result->status = newStatus.c_str();
        result->itemId = itemId;
        return result;
    } catch (...) {
        conn->rollback();
        throw;
    }
}

oatpp::Object<ItemDetailedResponse> ItemManageRepository::updateItem(int64_t itemId, const oatpp::Object<ItemUpdateRequest>& request) {
    auto conn = Database::getConnection();
    conn->setAutoCommit(false);
    try {
        std::unique_ptr<sql::PreparedStatement> selectStmt(conn->prepareStatement(
            "SELECT total_count, available_count FROM items WHERE id = ? FOR UPDATE"
        ));
        selectStmt->setInt64(1, itemId);
        std::unique_ptr<sql::ResultSet> rs(selectStmt->executeQuery());
        if (!rs->next()) throw std::runtime_error("物品不存在");
        int total = rs->getInt("total_count");
        int available = rs->getInt("available_count");
        int borrowed = total - available;
        int newTotal = request->totalCount ? request->totalCount.getValue(total) : total;
        if (newTotal < borrowed) throw std::runtime_error("总库存不能小于已占用库存");
        int newAvailable = newTotal - borrowed;

        std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
            "UPDATE items SET name = COALESCE(?, name), category = COALESCE(?, category), description = COALESCE(?, description), "
            "total_count = ?, available_count = ?, deposit = COALESCE(?, deposit) WHERE id = ?"
        ));
        if (request->name) stmt->setString(1, request->name->c_str()); else stmt->setNull(1, 0);
        if (request->category) stmt->setString(2, request->category->c_str()); else stmt->setNull(2, 0);
        if (request->description) stmt->setString(3, request->description->c_str()); else stmt->setNull(3, 0);
        stmt->setInt(4, newTotal);
        stmt->setInt(5, newAvailable);
        if (request->deposit) stmt->setDouble(6, request->deposit.getValue(0.0)); else stmt->setNull(6, 0);
        stmt->setInt64(7, itemId);
        stmt->executeUpdate();
        conn->commit();
        return getItemDetail(itemId);
    } catch (...) {
        conn->rollback();
        throw;
    }
}

oatpp::Object<StatusResultDto> ItemManageRepository::updateItemStatus(int64_t itemId, const std::string& status) {
    auto conn = Database::getConnection();
    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement("UPDATE items SET status = ? WHERE id = ?"));
    stmt->setString(1, status);
    stmt->setInt64(2, itemId);
    int affected = stmt->executeUpdate();
    if (affected == 0) throw std::runtime_error("物品不存在");

    auto result = StatusResultDto::createShared();
    result->id = itemId;
    result->status = status.c_str();
    return result;
}
