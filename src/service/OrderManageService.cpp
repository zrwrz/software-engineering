#include "service/OrderManageService.hpp"
#include "db/AuditLogRepository.hpp"
#include "db/OrderManageRepository.hpp"

#include <stdexcept>

oatpp::Object<OrderListResponse> OrderManageService::listOrders(int32_t page, int32_t size, const std::string& status) {
    return OrderManageRepository::listOrders(page, size, status);
}

oatpp::Object<OrderDto> OrderManageService::getOrderDetail(int64_t orderId) {
    if (orderId <= 0) throw std::runtime_error("订单ID不能为空");
    return OrderManageRepository::getOrderDetail(orderId);
}

oatpp::Object<StatusResultDto> OrderManageService::auditOrder(int64_t operatorId, int64_t orderId, const oatpp::Object<AuditRequest>& request) {
    if (!request || !request->action) throw std::runtime_error("审核动作不能为空");
    std::string action = request->action->c_str();
    if (action != "APPROVE" && action != "REJECT") throw std::runtime_error("审核动作不合法");
    std::string remark = request->remark ? request->remark->c_str() : "";
    auto result = OrderManageRepository::auditOrder(operatorId, orderId, action, remark);
    AuditLogRepository::createLog(operatorId, "ORDER_AUDIT", "order:" + std::to_string(orderId), "审核预约订单：" + action);
    return result;
}

oatpp::Object<StatusResultDto> OrderManageService::borrowOrder(int64_t operatorId, int64_t orderId) {
    auto result = OrderManageRepository::borrowOrder(operatorId, orderId);
    AuditLogRepository::createLog(operatorId, "ORDER_BORROW", "order:" + std::to_string(orderId), "办理借出");
    return result;
}

oatpp::Object<StatusResultDto> OrderManageService::returnOrder(int64_t operatorId, int64_t orderId, const oatpp::Object<ReturnOrderRequest>& request) {
    auto result = OrderManageRepository::returnOrder(operatorId, orderId, request);
    AuditLogRepository::createLog(operatorId, "ORDER_RETURN", "order:" + std::to_string(orderId), "办理归还");
    return result;
}

oatpp::Object<MarkOverdueResultDto> OrderManageService::markOverdue() {
    return OrderManageRepository::markOverdue();
}
