#ifndef CREDIT_REPOSITORY_HPP
#define CREDIT_REPOSITORY_HPP

#include "dto/entity/CreditAdjustResultDto.hpp"
#include "dto/request/CreditAdjustRequest.hpp"
#include "dto/response/CreditRecordListResponse.hpp"
#include <string>

/**
 * 信用积分数据访问层
 */
class CreditRepository {
  public:
    static oatpp::Object<CreditRecordListResponse> listRecords(int64_t userId, int32_t page, int32_t size);
    static oatpp::Object<CreditAdjustResultDto> adjustCredit(const oatpp::Object<CreditAdjustRequest>& request);
};

#endif
