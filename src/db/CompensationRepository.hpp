#ifndef COMPENSATION_REPOSITORY_HPP
#define COMPENSATION_REPOSITORY_HPP

#include "dto/entity/CompensationRecordDto.hpp"
#include "dto/entity/StatusResultDto.hpp"
#include "dto/request/CompensationCreateRequest.hpp"
#include "dto/response/CompensationRecordListResponse.hpp"
#include <string>

/**
 * 赔偿处理数据访问层
 */
class CompensationRepository {
  public:
    static oatpp::Object<CompensationRecordListResponse> listRecords(int32_t page, int32_t size, const std::string& status, int64_t userId, int64_t orderId);
    static oatpp::Object<CompensationRecordDto> createRecord(const oatpp::Object<CompensationCreateRequest>& request);
    static oatpp::Object<StatusResultDto> updateStatus(int64_t id, const std::string& status);
};

#endif
