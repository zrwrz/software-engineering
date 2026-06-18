#include "service/CompensationService.hpp"
#include "db/AuditLogRepository.hpp"
#include "db/CompensationRepository.hpp"

#include <stdexcept>

oatpp::Object<CompensationRecordListResponse> CompensationService::listRecords(int32_t page, int32_t size, const std::string& status, int64_t userId, int64_t orderId) {
    return CompensationRepository::listRecords(page, size, status, userId, orderId);
}

oatpp::Object<CompensationRecordDto> CompensationService::createRecord(int64_t operatorId, const oatpp::Object<CompensationCreateRequest>& request) {
    if (!request || !request->orderId || request->orderId.getValue(0) <= 0) throw std::runtime_error("订单ID不能为空");
    if (!request->userId || request->userId.getValue(0) <= 0) throw std::runtime_error("用户ID不能为空");
    if (!request->amount || request->amount.getValue(0.0) < 0) throw std::runtime_error("赔偿金额不能为负数");
    auto result = CompensationRepository::createRecord(request);
    AuditLogRepository::createLog(operatorId, "COMPENSATION_CREATE", "order:" + std::to_string(request->orderId.getValue(0)), "新增赔偿记录");
    return result;
}

oatpp::Object<StatusResultDto> CompensationService::updateStatus(int64_t operatorId, int64_t id, const oatpp::Object<StatusUpdateRequest>& request) {
    if (!request || !request->status) throw std::runtime_error("赔偿状态不能为空");
    std::string status = request->status->c_str();
    if (status != "PENDING" && status != "PAID" && status != "WAIVED") throw std::runtime_error("赔偿状态不合法");
    auto result = CompensationRepository::updateStatus(id, status);
    AuditLogRepository::createLog(operatorId, "COMPENSATION_STATUS_UPDATE", "compensation:" + std::to_string(id), "更新赔偿状态为 " + status);
    return result;
}
