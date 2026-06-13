#ifndef COMPENSATION_SERVICE_HPP
#define COMPENSATION_SERVICE_HPP

#include "dto/entity/CompensationRecordDto.hpp"
#include "dto/entity/StatusResultDto.hpp"
#include "dto/request/CompensationCreateRequest.hpp"
#include "dto/request/StatusUpdateRequest.hpp"
#include "dto/response/CompensationRecordListResponse.hpp"
#include <string>

/**
 * 赔偿处理业务服务
 */
class CompensationService {
  public:
    static oatpp::Object<CompensationRecordListResponse> listRecords(int32_t page, int32_t size, const std::string& status, int64_t userId, int64_t orderId);
    static oatpp::Object<CompensationRecordDto> createRecord(int64_t operatorId, const oatpp::Object<CompensationCreateRequest>& request);
    static oatpp::Object<StatusResultDto> updateStatus(int64_t operatorId, int64_t id, const oatpp::Object<StatusUpdateRequest>& request);
};

#endif
