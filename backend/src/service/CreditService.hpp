#ifndef CREDIT_SERVICE_HPP
#define CREDIT_SERVICE_HPP

#include "dto/entity/CreditAdjustResultDto.hpp"
#include "dto/request/CreditAdjustRequest.hpp"
#include "dto/response/CreditRecordListResponse.hpp"

/**
 * 信用积分业务服务
 */
class CreditService {
  public:
    static oatpp::Object<CreditRecordListResponse> listRecords(int64_t currentUserId, const std::string& currentRole, int64_t userId, int32_t page, int32_t size);
    static oatpp::Object<CreditAdjustResultDto> adjustCredit(int64_t operatorId, const oatpp::Object<CreditAdjustRequest>& request);
};

#endif
