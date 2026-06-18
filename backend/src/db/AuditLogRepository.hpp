#ifndef AUDIT_LOG_REPOSITORY_HPP
#define AUDIT_LOG_REPOSITORY_HPP

#include "dto/entity/AuditLogDto.hpp"
#include "dto/response/AuditLogListResponse.hpp"
#include <cppconn/connection.h>
#include <string>

/**
 * 审计日志数据访问层
 */
class AuditLogRepository {
  public:
    static void createLog(int64_t operatorId, const std::string& action, const std::string& target, const std::string& detail);
    static void createLog(sql::Connection* conn, int64_t operatorId, const std::string& action, const std::string& target, const std::string& detail);
    static oatpp::Object<AuditLogListResponse> listLogs(int32_t page, int32_t size, int64_t operatorId, const std::string& startTime, const std::string& endTime);
};

#endif
