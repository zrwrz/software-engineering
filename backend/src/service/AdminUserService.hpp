#ifndef ADMIN_USER_SERVICE_HPP
#define ADMIN_USER_SERVICE_HPP

#include "dto/entity/StatusResultDto.hpp"
#include "dto/response/AdminUserListResponse.hpp"
#include "dto/response/AuditLogListResponse.hpp"
#include <string>

/**
 * 管理端用户业务服务
 */
class AdminUserService {
  public:
    static oatpp::Object<AdminUserListResponse> listUsers(int32_t page, int32_t size, const std::string& keyword, const std::string& status);
    static oatpp::Object<StatusResultDto> updateUserStatus(int64_t operatorId, int64_t userId, const std::string& status);
    static oatpp::Object<AuditLogListResponse> listAuditLogs(int32_t page, int32_t size, int64_t operatorId, const std::string& startTime, const std::string& endTime);
};

#endif
