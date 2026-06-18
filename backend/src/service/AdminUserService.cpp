#include "service/AdminUserService.hpp"
#include "db/AdminUserRepository.hpp"
#include "db/AuditLogRepository.hpp"

#include <stdexcept>

oatpp::Object<AdminUserListResponse> AdminUserService::listUsers(int32_t page, int32_t size, const std::string& keyword, const std::string& status) {
    std::string dbStatus = status;
    if (status == "ACTIVE") dbStatus = "active";
    if (status == "DISABLED") dbStatus = "frozen";
    if (status == "BANNED") dbStatus = "banned";
    if (status == "FROZEN") dbStatus = "frozen";
    return AdminUserRepository::listUsers(page, size, keyword, dbStatus);
}

oatpp::Object<StatusResultDto> AdminUserService::updateUserStatus(int64_t operatorId, int64_t userId, const std::string& status) {
    if (userId <= 0) throw std::runtime_error("用户ID不能为空");
    if (status.empty()) throw std::runtime_error("用户状态不能为空");

    // 兼容接口文档中的 ACTIVE/DISABLED 与 SQL 文件中的 active/frozen/banned 状态值。
    std::string dbStatus = status;
    if (status == "ACTIVE") dbStatus = "active";
    if (status == "DISABLED") dbStatus = "frozen";
    if (status == "BANNED") dbStatus = "banned";
    if (status == "FROZEN") dbStatus = "frozen";
    if (dbStatus != "active" && dbStatus != "banned" && dbStatus != "frozen") {
        throw std::runtime_error("用户状态不合法");
    }

    auto result = AdminUserRepository::updateUserStatus(userId, dbStatus);
    AuditLogRepository::createLog(operatorId, "USER_STATUS_UPDATE", "user:" + std::to_string(userId), "更新用户状态为 " + dbStatus);
    return result;
}

oatpp::Object<AuditLogListResponse> AdminUserService::listAuditLogs(int32_t page, int32_t size, int64_t operatorId, const std::string& startTime, const std::string& endTime) {
    return AuditLogRepository::listLogs(page, size, operatorId, startTime, endTime);
}
