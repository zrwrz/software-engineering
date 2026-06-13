#include "db/OrderManageRepository.hpp"
#include "db/Database.hpp"

#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <stdexcept>

namespace {
int normalizePage(int32_t page) { return page <= 0 ? 1 : page; }
int normalizeSize(int32_t size) { return size <= 0 ? 20 : size; }

oatpp::Object<OrderDto> readOrderDto(sql::ResultSet* rs) {
    auto order = OrderDto::createShared();
    order->id = rs->getInt64("id");
    order->orderNo = rs->getString("order_no").c_str();
    order->userId = rs->getInt64("user_id");
    order->itemId = rs->getInt64("item_id");
    order->itemName = rs->isNull("item_name") ? nullptr : oatpp::String(rs->getString("item_name").c_str());
    order->quantity = rs->getInt("quantity");
    order->startTime = rs->getString("reserve_start_time").c_str();
    order->endTime = rs->getString("reserve_end_time").c_str();
    order->pickupTime = rs->isNull("pickup_time") ? nullptr : oatpp::String(rs->getString("pickup_time").c_str());
    order->returnTime = rs->isNull("return_time") ? nullptr : oatpp::String(rs->getString("return_time").c_str());
    order->purpose = rs->isNull("remark") ? nullptr : oatpp::String(rs->getString("remark").c_str());
    order->status = rs->getString("status").c_str();
    order->remark = rs->isNull("review_remark") ? nullptr : oatpp::String(rs->getString("review_remark").c_str());
    order->createdBy = rs->getInt64("user_id");
    order->createdAt = rs->getString("created_at").c_str();
    order->updatedAt = rs->isNull("updated_at") ? nullptr : oatpp::String(rs->getString("updated_at").c_str());
    return order;
}
}

oatpp::Object<OrderListResponse> OrderManageRepository::listOrders(int32_t page, int32_t size, const std::string& status) {
    page = normalizePage(page);
    size = normalizeSize(size);
    int offset = (page - 1) * size;
    auto conn = Database::getConnection();
    std::string where = " WHERE 1=1 ";
    if (!status.empty()) where += " AND o.status = ? ";

    std::unique_ptr<sql::PreparedStatement> countStmt(conn->prepareStatement("SELECT COUNT(*) total FROM orders o" + where));
    if (!status.empty()) countStmt->setString(1, status);
    std::unique_ptr<sql::ResultSet> countRs(countStmt->executeQuery());
    int64_t total = 0;
    if (countRs->next()) total = countRs->getInt64("total");

    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "SELECT o.id, o.order_no, o.user_id, o.item_id, i.name item_name, o.quantity, "
        "DATE_FORMAT(o.reserve_start_time, '%Y-%m-%d %H:%i:%s') reserve_start_time, "
        "DATE_FORMAT(o.reserve_end_time, '%Y-%m-%d %H:%i:%s') reserve_end_time, "
        "DATE_FORMAT(o.pickup_time, '%Y-%m-%d %H:%i:%s') pickup_time, "
        "DATE_FORMAT(o.return_time, '%Y-%m-%d %H:%i:%s') return_time, o.status, o.remark, o.review_remark, "
        "DATE_FORMAT(o.created_at, '%Y-%m-%d %H:%i:%s') created_at, DATE_FORMAT(o.updated_at, '%Y-%m-%d %H:%i:%s') updated_at "
        "FROM orders o LEFT JOIN items i ON o.item_id = i.id" + where + " ORDER BY o.created_at DESC LIMIT ? OFFSET ?"
    ));
    int index = 1;
    if (!status.empty()) stmt->setString(index++, status);
    stmt->setInt(index++, size);
    stmt->setInt(index++, offset);

    auto response = OrderListResponse::createShared();
    response->list = oatpp::Vector<oatpp::Object<OrderDto>>::createShared();
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    while (rs->next()) {
        response->list->push_back(readOrderDto(rs.get()));
    }
    response->page = page;
    response->size = size;
    response->total = total;
    return response;
}

oatpp::Object<OrderDto> OrderManageRepository::getOrderDetail(int64_t orderId) {
    auto conn = Database::getConnection();
    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "SELECT o.id, o.order_no, o.user_id, o.item_id, i.name item_name, o.quantity, "
        "DATE_FORMAT(o.reserve_start_time, '%Y-%m-%d %H:%i:%s') reserve_start_time, "
        "DATE_FORMAT(o.reserve_end_time, '%Y-%m-%d %H:%i:%s') reserve_end_time, "
        "DATE_FORMAT(o.pickup_time, '%Y-%m-%d %H:%i:%s') pickup_time, "
        "DATE_FORMAT(o.return_time, '%Y-%m-%d %H:%i:%s') return_time, o.status, o.remark, o.review_remark, "
        "DATE_FORMAT(o.created_at, '%Y-%m-%d %H:%i:%s') created_at, DATE_FORMAT(o.updated_at, '%Y-%m-%d %H:%i:%s') updated_at "
        "FROM orders o LEFT JOIN items i ON o.item_id = i.id WHERE o.id = ?"
    ));
    stmt->setInt64(1, orderId);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) throw std::runtime_error("订单不存在");
    return readOrderDto(rs.get());
}

oatpp::Object<StatusResultDto> OrderManageRepository::auditOrder(int64_t operatorId, int64_t orderId, const std::string& action, const std::string& remark) {
    auto conn = Database::getConnection();
    conn->setAutoCommit(false);
    try {
        std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
            "SELECT o.status, o.quantity, o.item_id, i.available_count, i.status item_status FROM orders o JOIN items i ON o.item_id = i.id WHERE o.id = ? FOR UPDATE"
        ));
        stmt->setInt64(1, orderId);
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
        if (!rs->next()) throw std::runtime_error("订单不存在");
        if (rs->getString("status") != "CREATED") throw std::runtime_error("订单状态不允许当前操作");

        std::string newStatus;
        if (action == "APPROVE") {
            if (rs->getString("item_status") != "ON_SHELF" || rs->getInt("available_count") < rs->getInt("quantity")) {
                throw std::runtime_error("库存不足或物品不可预约");
            }
            newStatus = "APPROVED";
            std::unique_ptr<sql::PreparedStatement> updateItem(conn->prepareStatement(
                "UPDATE items SET available_count = available_count - ? WHERE id = ?"
            ));
            updateItem->setInt(1, rs->getInt("quantity"));
            updateItem->setInt64(2, rs->getInt64("item_id"));
            updateItem->executeUpdate();
        } else if (action == "REJECT") {
            newStatus = "REJECTED";
        } else {
            throw std::runtime_error("审核动作不合法");
        }

        std::unique_ptr<sql::PreparedStatement> updateOrder(conn->prepareStatement(
            "UPDATE orders SET status = ?, review_remark = ?, reviewed_by = ?, reviewed_at = NOW() WHERE id = ?"
        ));
        updateOrder->setString(1, newStatus);
        updateOrder->setString(2, remark);
        updateOrder->setInt64(3, operatorId);
        updateOrder->setInt64(4, orderId);
        updateOrder->executeUpdate();
        conn->commit();

        auto result = StatusResultDto::createShared();
        result->id = orderId;
        result->status = newStatus.c_str();
        result->remark = remark.c_str();
        return result;
    } catch (...) {
        conn->rollback();
        throw;
    }
}

oatpp::Object<StatusResultDto> OrderManageRepository::borrowOrder(int64_t operatorId, int64_t orderId) {
    auto conn = Database::getConnection();
    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "UPDATE orders SET status = 'BORROWED', pickup_time = NOW(), borrowed_by = ? WHERE id = ? AND status = 'APPROVED'"
    ));
    stmt->setInt64(1, operatorId);
    stmt->setInt64(2, orderId);
    int affected = stmt->executeUpdate();
    if (affected == 0) throw std::runtime_error("订单不存在或状态不允许办理借出");

    std::unique_ptr<sql::PreparedStatement> query(conn->prepareStatement(
        "SELECT DATE_FORMAT(pickup_time, '%Y-%m-%d %H:%i:%s') pickup_time FROM orders WHERE id = ?"
    ));
    query->setInt64(1, orderId);
    std::unique_ptr<sql::ResultSet> rs(query->executeQuery());

    auto result = StatusResultDto::createShared();
    result->id = orderId;
    result->status = "BORROWED";
    if (rs->next()) result->pickupTime = rs->getString("pickup_time").c_str();
    return result;
}

oatpp::Object<StatusResultDto> OrderManageRepository::returnOrder(int64_t operatorId, int64_t orderId, const oatpp::Object<ReturnOrderRequest>& request) {
    auto conn = Database::getConnection();
    conn->setAutoCommit(false);
    try {
        std::unique_ptr<sql::PreparedStatement> selectStmt(conn->prepareStatement(
            "SELECT status, quantity, item_id, user_id FROM orders WHERE id = ? FOR UPDATE"
        ));
        selectStmt->setInt64(1, orderId);
        std::unique_ptr<sql::ResultSet> rs(selectStmt->executeQuery());
        if (!rs->next()) throw std::runtime_error("订单不存在");
        std::string oldStatus = rs->getString("status");
        if (oldStatus != "BORROWED" && oldStatus != "OVERDUE") throw std::runtime_error("订单状态不允许办理归还");
        int quantity = rs->getInt("quantity");
        int64_t itemId = rs->getInt64("item_id");
        int64_t userId = rs->getInt64("user_id");

        std::unique_ptr<sql::PreparedStatement> updateOrder(conn->prepareStatement(
            "UPDATE orders SET status = 'RETURNED', return_time = NOW(), return_remark = ?, returned_by = ? WHERE id = ?"
        ));
        updateOrder->setString(1, request && request->returnRemark ? request->returnRemark->c_str() : "");
        updateOrder->setInt64(2, operatorId);
        updateOrder->setInt64(3, orderId);
        updateOrder->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> updateItem(conn->prepareStatement(
            "UPDATE items SET available_count = LEAST(total_count, available_count + ?) WHERE id = ?"
        ));
        updateItem->setInt(1, quantity);
        updateItem->setInt64(2, itemId);
        updateItem->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> recordStmt(conn->prepareStatement(
            "INSERT INTO borrow_records(order_id, item_id, user_id, operator_id, action, remark) VALUES(?, ?, ?, ?, 'RETURN', ?)"
        ));
        recordStmt->setInt64(1, orderId);
        recordStmt->setInt64(2, itemId);
        recordStmt->setInt64(3, userId);
        recordStmt->setInt64(4, operatorId);
        recordStmt->setString(5, request && request->returnRemark ? request->returnRemark->c_str() : "");
        recordStmt->executeUpdate();

        if (oldStatus == "OVERDUE") {
            std::unique_ptr<sql::PreparedStatement> creditStmt(conn->prepareStatement(
                "INSERT INTO credit_records(user_id, order_id, change_value, reason) VALUES(?, ?, -5, '逾期归还')"
            ));
            creditStmt->setInt64(1, userId);
            creditStmt->setInt64(2, orderId);
            creditStmt->executeUpdate();
            std::unique_ptr<sql::PreparedStatement> userStmt(conn->prepareStatement(
                "UPDATE users SET credit_score = credit_score - 5 WHERE id = ?"
            ));
            userStmt->setInt64(1, userId);
            userStmt->executeUpdate();
        }

        std::unique_ptr<sql::PreparedStatement> timeStmt(conn->prepareStatement(
            "SELECT DATE_FORMAT(return_time, '%Y-%m-%d %H:%i:%s') return_time FROM orders WHERE id = ?"
        ));
        timeStmt->setInt64(1, orderId);
        std::unique_ptr<sql::ResultSet> timeRs(timeStmt->executeQuery());

        conn->commit();
        auto result = StatusResultDto::createShared();
        result->id = orderId;
        result->status = "RETURNED";
        if (timeRs->next()) result->returnTime = timeRs->getString("return_time").c_str();
        result->needCompensation = request && request->needCompensation ? request->needCompensation.getValue(false) : false;
        return result;
    } catch (...) {
        conn->rollback();
        throw;
    }
}

oatpp::Object<MarkOverdueResultDto> OrderManageRepository::markOverdue() {
    auto conn = Database::getConnection();
    conn->setAutoCommit(false);
    try {
        std::unique_ptr<sql::PreparedStatement> selectStmt(conn->prepareStatement(
            "SELECT id, user_id FROM orders WHERE status = 'BORROWED' AND reserve_end_time < NOW() FOR UPDATE"
        ));
        std::unique_ptr<sql::ResultSet> rs(selectStmt->executeQuery());
        auto result = MarkOverdueResultDto::createShared();
        result->orderIds = oatpp::Vector<oatpp::Int64>::createShared();
        while (rs->next()) {
            int64_t orderId = rs->getInt64("id");
            int64_t userId = rs->getInt64("user_id");
            result->orderIds->push_back(orderId);
            std::unique_ptr<sql::PreparedStatement> noticeStmt(conn->prepareStatement(
                "INSERT INTO notifications(user_id, type, title, content, related_order_id) VALUES(?, 'OVERDUE', '逾期提醒', '您的借用订单已逾期，请尽快归还。', ?)"
            ));
            noticeStmt->setInt64(1, userId);
            noticeStmt->setInt64(2, orderId);
            noticeStmt->executeUpdate();
        }

        if (!result->orderIds->empty()) {
            std::unique_ptr<sql::PreparedStatement> updateStmt(conn->prepareStatement(
                "UPDATE orders SET status = 'OVERDUE' WHERE status = 'BORROWED' AND reserve_end_time < NOW()"
            ));
            updateStmt->executeUpdate();
        }
        result->updatedCount = static_cast<v_int32>(result->orderIds->size());
        conn->commit();
        return result;
    } catch (...) {
        conn->rollback();
        throw;
    }
}
