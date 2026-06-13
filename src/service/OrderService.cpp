#include "service/OrderService.hpp"

#include "db/AuditLogRepository.hpp"
#include "db/Database.hpp"
#include "db/OrderManageRepository.hpp"
#include "dto/request/ReturnOrderRequest.hpp"

#include <cppconn/datatype.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {
int normalizePage(int32_t page) { return page <= 0 ? 1 : page; }
int normalizeSize(int32_t size) { return size <= 0 ? 10 : size; }

std::string toStdString(const oatpp::String& value) {
    return value ? std::string(value->c_str()) : std::string();
}

std::string nextOrderNo() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    return "R" + std::to_string(static_cast<long long>(millis));
}

void requireReservationTime(const oatpp::String& startTime, const oatpp::String& endTime) {
    if (!startTime || startTime->empty() || !endTime || endTime->empty()) {
        throw std::runtime_error("预约时间不能为空");
    }
    if (std::string(startTime->c_str()) >= std::string(endTime->c_str())) {
        throw std::runtime_error("预约开始时间必须早于结束时间");
    }
}

oatpp::Object<ReservationListItemDto> mapReservationListItem(sql::ResultSet* rs) {
    auto item = ReservationListItemDto::createShared();
    item->id = rs->getInt64("id");
    item->orderNo = rs->getString("order_no").c_str();
    item->userId = rs->getInt64("user_id");
    item->itemId = rs->getInt64("item_id");
    item->quantity = rs->getInt("quantity");
    item->status = rs->getString("status").c_str();
    item->createdAt = rs->getString("created_at").c_str();
    return item;
}

oatpp::Object<ReservationDetailedResponse> mapReservationDetail(sql::ResultSet* rs) {
    auto dto = ReservationDetailedResponse::createShared();
    dto->id = rs->getInt64("id");
    dto->orderNo = rs->getString("order_no").c_str();
    dto->userId = rs->getInt64("user_id");
    dto->itemId = rs->getInt64("item_id");
    dto->quantity = rs->getInt("quantity");
    dto->reserveStartTime = rs->getString("reserve_start_time").c_str();
    dto->reserveEndTime = rs->getString("reserve_end_time").c_str();
    dto->pickupTime = rs->isNull("pickup_time") ? nullptr : oatpp::String(rs->getString("pickup_time").c_str());
    dto->returnTime = rs->isNull("return_time") ? nullptr : oatpp::String(rs->getString("return_time").c_str());
    dto->status = rs->getString("status").c_str();
    dto->remark = rs->isNull("remark") ? nullptr : oatpp::String(rs->getString("remark").c_str());
    dto->createdAt = rs->getString("created_at").c_str();
    return dto;
}

oatpp::Object<ReservationListResponse> queryOrders(int64_t userId, int32_t page, int32_t size, const std::string& status) {
    page = normalizePage(page);
    size = normalizeSize(size);
    int offset = (page - 1) * size;

    auto conn = Database::getConnection();
    std::string where = " WHERE 1=1 ";
    if (userId > 0) where += " AND user_id = ? ";
    if (!status.empty()) where += " AND status = ? ";

    std::unique_ptr<sql::PreparedStatement> countStmt(conn->prepareStatement("SELECT COUNT(*) total FROM orders" + where));
    int index = 1;
    if (userId > 0) countStmt->setInt64(index++, userId);
    if (!status.empty()) countStmt->setString(index++, status);
    std::unique_ptr<sql::ResultSet> countRs(countStmt->executeQuery());
    int64_t total = 0;
    if (countRs->next()) total = countRs->getInt64("total");

    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "SELECT id, order_no, user_id, item_id, quantity, status, DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') created_at "
        "FROM orders" + where + " ORDER BY created_at DESC LIMIT ? OFFSET ?"
    ));
    index = 1;
    if (userId > 0) stmt->setInt64(index++, userId);
    if (!status.empty()) stmt->setString(index++, status);
    stmt->setInt(index++, size);
    stmt->setInt(index++, offset);

    auto response = ReservationListResponse::createShared();
    response->list = oatpp::List<oatpp::Object<ReservationListItemDto>>::createShared();
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    while (rs->next()) {
        response->list->push_back(mapReservationListItem(rs.get()));
    }
    response->page = page;
    response->size = size;
    response->total = total;
    return response;
}
}

oatpp::Object<CreateReservationResponse> OrderService::createReservation(int64_t userId, const oatpp::Object<CreateReservationRequest>& request) {
    if (userId <= 0) {
        throw std::runtime_error("用户ID不能为空");
    }
    if (!request) {
        throw std::runtime_error("请求体不能为空");
    }
    if (!request->itemId || request->itemId.getValue(0) <= 0) {
        throw std::runtime_error("物品ID不能为空");
    }
    if (!request->quantity || request->quantity.getValue(0) <= 0) {
        throw std::runtime_error("预约数量必须大于0");
    }
    requireReservationTime(request->reserveStartTime, request->reserveEndTime);

    auto conn = Database::getConnection();
    conn->setAutoCommit(false);
    try {
        std::unique_ptr<sql::PreparedStatement> itemStmt(conn->prepareStatement(
            "SELECT available_count, status FROM items WHERE id = ? FOR UPDATE"
        ));
        itemStmt->setInt64(1, request->itemId.getValue(0));
        std::unique_ptr<sql::ResultSet> itemRs(itemStmt->executeQuery());
        if (!itemRs->next()) {
            throw std::runtime_error("物品不存在");
        }
        if (itemRs->getString("status") != "ON_SHELF" || itemRs->getInt("available_count") < request->quantity.getValue(0)) {
            throw std::runtime_error("库存不足或物品不可预约");
        }

        std::unique_ptr<sql::PreparedStatement> creditCheck(conn->prepareStatement(
            "SELECT credit_score, status FROM users WHERE id = ? FOR UPDATE"
        ));
        creditCheck->setInt64(1, userId);
        std::unique_ptr<sql::ResultSet> userRs(creditCheck->executeQuery());
        if (!userRs->next()) throw std::runtime_error("用户不存在");
        if (userRs->getString("status") != "active") throw std::runtime_error("账号状态不允许预约");
        if (userRs->getInt("credit_score") < 60) throw std::runtime_error("信用积分不足，暂不可预约");

        std::string orderNo = nextOrderNo();
        std::unique_ptr<sql::PreparedStatement> insertStmt(conn->prepareStatement(
            "INSERT INTO orders(order_no, user_id, item_id, quantity, reserve_start_time, reserve_end_time, status, remark, created_by) "
            "VALUES(?, ?, ?, ?, ?, ?, 'CREATED', ?, ?)"
        ));
        insertStmt->setString(1, orderNo);
        insertStmt->setInt64(2, userId);
        insertStmt->setInt64(3, request->itemId.getValue(0));
        insertStmt->setInt(4, request->quantity.getValue(0));
        insertStmt->setString(5, request->reserveStartTime->c_str());
        insertStmt->setString(6, request->reserveEndTime->c_str());
        insertStmt->setString(7, request->remark ? request->remark->c_str() : "");
        insertStmt->setInt64(8, userId);
        insertStmt->executeUpdate();

        std::unique_ptr<sql::Statement> idStmt(conn->createStatement());
        std::unique_ptr<sql::ResultSet> idRs(idStmt->executeQuery("SELECT LAST_INSERT_ID() id"));
        int64_t orderId = 0;
        if (idRs->next()) orderId = idRs->getInt64("id");

        std::unique_ptr<sql::PreparedStatement> query(conn->prepareStatement(
            "SELECT id, order_no, user_id, item_id, quantity, "
            "DATE_FORMAT(reserve_start_time, '%Y-%m-%d %H:%i:%s') reserve_start_time, "
            "DATE_FORMAT(reserve_end_time, '%Y-%m-%d %H:%i:%s') reserve_end_time, status, "
            "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') created_at FROM orders WHERE id = ?"
        ));
        query->setInt64(1, orderId);
        std::unique_ptr<sql::ResultSet> rs(query->executeQuery());
        if (!rs->next()) {
            throw std::runtime_error("预约创建失败");
        }

        std::unique_ptr<sql::PreparedStatement> noticeStmt(conn->prepareStatement(
            "INSERT INTO notifications(user_id, type, title, content, related_order_id) "
            "SELECT id, 'RESERVATION', '新的预约申请', '有新的物品预约申请待审核。', ? FROM users WHERE role IN ('resource_admin', 'system_admin') AND status = 'active'"
        ));
        noticeStmt->setInt64(1, orderId);
        noticeStmt->executeUpdate();

        AuditLogRepository::createLog(userId, "ORDER_CREATE", "order:" + std::to_string(orderId), "提交预约申请");

        conn->commit();
        auto response = CreateReservationResponse::createShared();
        response->id = rs->getInt64("id");
        response->orderNo = rs->getString("order_no").c_str();
        response->userId = rs->getInt64("user_id");
        response->itemId = rs->getInt64("item_id");
        response->quantity = rs->getInt("quantity");
        response->reserveStartTime = rs->getString("reserve_start_time").c_str();
        response->reserveEndTime = rs->getString("reserve_end_time").c_str();
        response->status = rs->getString("status").c_str();
        response->createdAt = rs->getString("created_at").c_str();
        return response;
    } catch (...) {
        conn->rollback();
        throw;
    }
}

oatpp::Object<ReservationListResponse> OrderService::getMyOrders(int64_t userId, const oatpp::Object<ReservationListRequest>& request) {
    int32_t page = request && request->page ? request->page.getValue(1) : 1;
    int32_t size = request && request->size ? request->size.getValue(10) : 10;
    std::string status = request && request->status ? request->status->c_str() : "";
    return getMyOrders(userId, page, size, status);
}

oatpp::Object<ReservationListResponse> OrderService::getMyOrders(int64_t userId, int32_t page, int32_t size, const std::string& status) {
    if (userId <= 0) {
        throw std::runtime_error("用户ID不能为空");
    }
    return queryOrders(userId, page, size, status);
}

oatpp::Object<ReservationListResponse> OrderService::getOrderList(const oatpp::Object<ReservationListRequest>& request) {
    int32_t page = request && request->page ? request->page.getValue(1) : 1;
    int32_t size = request && request->size ? request->size.getValue(20) : 20;
    std::string status = request && request->status ? request->status->c_str() : "";
    return getOrderList(page, size, status);
}

oatpp::Object<ReservationListResponse> OrderService::getOrderList(int32_t page, int32_t size, const std::string& status) {
    return queryOrders(0, page, size, status);
}

oatpp::Object<ReservationDetailedResponse> OrderService::getOrderDetailed(int64_t orderId) {
    if (orderId <= 0) {
        throw std::runtime_error("订单ID不能为空");
    }
    auto conn = Database::getConnection();
    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "SELECT id, order_no, user_id, item_id, quantity, "
        "DATE_FORMAT(reserve_start_time, '%Y-%m-%d %H:%i:%s') reserve_start_time, "
        "DATE_FORMAT(reserve_end_time, '%Y-%m-%d %H:%i:%s') reserve_end_time, "
        "DATE_FORMAT(pickup_time, '%Y-%m-%d %H:%i:%s') pickup_time, "
        "DATE_FORMAT(return_time, '%Y-%m-%d %H:%i:%s') return_time, status, remark, "
        "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i:%s') created_at FROM orders WHERE id = ?"
    ));
    stmt->setInt64(1, orderId);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) {
        throw std::runtime_error("订单不存在");
    }
    return mapReservationDetail(rs.get());
}

oatpp::Object<CancelReservationResponse> OrderService::cancelReservation(int64_t currentUserId, const std::string& currentRole, int64_t orderId) {
    if (currentUserId <= 0) {
        throw std::runtime_error("用户ID不能为空");
    }
    auto order = getOrderDetailed(orderId);
    if (currentRole == "borrower" && order->userId.getValue(0) != currentUserId) {
        throw std::runtime_error("无权限操作该订单");
    }
    if (order->status != "CREATED") {
        throw std::runtime_error("当前订单状态不允许取消");
    }

    auto conn = Database::getConnection();
    std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(
        "UPDATE orders SET status = 'CANCELLED' WHERE id = ? AND status = 'CREATED'"
    ));
    stmt->setInt64(1, orderId);
    if (stmt->executeUpdate() == 0) {
        throw std::runtime_error("当前订单状态不允许取消");
    }

    auto response = CancelReservationResponse::createShared();
    response->id = orderId;
    response->status = "CANCELLED";
    return response;
}

oatpp::Object<AuditReservationResponse> OrderService::auditReservation(int64_t operatorId, int64_t orderId, const oatpp::Object<AuditReservationRequest>& request) {
    if (operatorId <= 0) {
        throw std::runtime_error("操作人ID不能为空");
    }
    if (orderId <= 0) {
        throw std::runtime_error("订单ID不能为空");
    }
    if (!request || !request->action) {
        throw std::runtime_error("审核动作不能为空");
    }
    std::string action = toStdString(request->action);
    if (action != "APPROVE" && action != "REJECT") {
        throw std::runtime_error("审核动作不合法");
    }
    std::string remark = request->reason ? request->reason->c_str() : "";
    auto result = OrderManageRepository::auditOrder(operatorId, orderId, action, remark);
    AuditLogRepository::createLog(operatorId, "ORDER_AUDIT", "order:" + std::to_string(orderId), "审核预约订单：" + action);

    auto response = AuditReservationResponse::createShared();
    response->id = result->id;
    response->status = result->status;
    response->remark = result->remark;
    return response;
}

oatpp::Object<RegisterBorrowResponse> OrderService::registerBorrow(int64_t operatorId, int64_t orderId) {
    if (operatorId <= 0) {
        throw std::runtime_error("操作人ID不能为空");
    }
    auto result = OrderManageRepository::borrowOrder(operatorId, orderId);
    AuditLogRepository::createLog(operatorId, "ORDER_BORROW", "order:" + std::to_string(orderId), "办理借出");

    auto response = RegisterBorrowResponse::createShared();
    response->id = result->id;
    response->status = result->status;
    response->pickupTime = result->pickupTime;
    return response;
}

oatpp::Object<RegisterReturnResponse> OrderService::registerReturn(int64_t operatorId, int64_t orderId, const oatpp::Object<RegisterReturnRequest>& request) {
    if (operatorId <= 0) {
        throw std::runtime_error("操作人ID不能为空");
    }
    auto returnRequest = ReturnOrderRequest::createShared();
    if (request) {
        returnRequest->returnRemark = request->returnRemark;
        returnRequest->needCompensation = request->needCompensation;
    }
    auto result = OrderManageRepository::returnOrder(operatorId, orderId, returnRequest);
    AuditLogRepository::createLog(operatorId, "ORDER_RETURN", "order:" + std::to_string(orderId), "办理归还");

    auto response = RegisterReturnResponse::createShared();
    response->id = result->id;
    response->status = result->status;
    response->returnTime = result->returnTime;
    response->needCompensation = result->needCompensation;
    return response;
}

oatpp::Object<MarkOverdueResponse> OrderService::markOverdue() {
    auto result = OrderManageRepository::markOverdue();
    auto response = MarkOverdueResponse::createShared();
    response->updatedCount = result->updatedCount;
    response->orderIds = oatpp::List<oatpp::Int64>::createShared();
    if (result->orderIds) {
        for (const auto& id : *result->orderIds) {
            response->orderIds->push_back(id);
        }
    }
    return response;
}
