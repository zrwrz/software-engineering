#include "service/CreditService.hpp"
#include "db/AuditLogRepository.hpp"
#include "db/CreditRepository.hpp"

#include <stdexcept>

oatpp::Object<CreditRecordListResponse> CreditService::listRecords(int64_t currentUserId, const std::string& currentRole, int64_t userId, int32_t page, int32_t size) {
    if (currentRole == "borrower") {
        userId = currentUserId;
    }
    return CreditRepository::listRecords(userId, page, size);
}

oatpp::Object<CreditAdjustResultDto> CreditService::adjustCredit(int64_t operatorId, const oatpp::Object<CreditAdjustRequest>& request) {
    if (!request || !request->userId || request->userId.getValue(0) <= 0) throw std::runtime_error("用户ID不能为空");
    if (!request->changeValue || request->changeValue.getValue(0) == 0) throw std::runtime_error("信用分变更值不能为空");
    if (!request->reason || request->reason->empty()) throw std::runtime_error("调整原因不能为空");
    auto result = CreditRepository::adjustCredit(request);
    AuditLogRepository::createLog(operatorId, "CREDIT_ADJUST", "user:" + std::to_string(request->userId.getValue(0)), "手工调整信用分");
    return result;
}
