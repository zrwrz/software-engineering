#include "service/AdminUserService.hpp"
#include "db/AdminUserRepository.hpp"
#include "db/AuditLogRepository.hpp"

#include <stdexcept>

oatpp::Object<AdminUserListResponse> AdminUserService::listUsers(int32_t page, int32_t size, const std::string& keyword, const std::string& status) {
    return AdminUserRepository::listUsers(page, size, keyword, status);
}

oatpp::Object<StatusResultDto> AdminUserService::updateUserStatus(int64_t operatorId, int64_t userId, const std::string& status) {
    if (userId <= 0) throw std::runtime_error("用户ID不能为空");
    if (status.empty()) throw std::runtime_error("用户状态不能为空");
    auto result = AdminUserRepository::updateUserStatus(userId, status);
    AuditLogRepository::createLog(operatorId, "USER_STATUS_UPDATE", "user:" + std::to_string(userId), "更新用户状态为 " + status);
    return result;
}

oatpp::Object<AuditLogListResponse> AdminUserService::listAuditLogs(int32_t page, int32_t size, int64_t operatorId, const std::string& startTime, const std::string& endTime) {
    return AuditLogRepository::listLogs(page, size, operatorId, startTime, endTime);
}
