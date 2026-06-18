#ifndef ITEM_MANAGE_REPOSITORY_HPP
#define ITEM_MANAGE_REPOSITORY_HPP

#include "dto/entity/ItemSubmissionDto.hpp"
#include "dto/entity/StatusResultDto.hpp"
#include "dto/request/ItemSubmissionRequest.hpp"
#include "dto/request/ItemUpdateRequest.hpp"
#include "dto/response/ItemDetailedResponse.hpp"
#include "dto/response/ItemListResponse.hpp"
#include "dto/response/ItemSubmissionListResponse.hpp"
#include <string>

/**
 * 物品管理数据访问层
 */
class ItemManageRepository {
  public:
    static oatpp::Object<ItemListResponse> listItems(int32_t page, int32_t size, const std::string& keyword, const std::string& category, const std::string& status);
    static oatpp::Object<ItemDetailedResponse> getItemDetail(int64_t itemId);
    static oatpp::Object<StatusResultDto> createSubmission(int64_t submitterId, const oatpp::Object<ItemSubmissionRequest>& request);
    static oatpp::Object<ItemSubmissionListResponse> listSubmissions(int32_t page, int32_t size, const std::string& status);
    static oatpp::Object<StatusResultDto> auditSubmission(int64_t operatorId, int64_t submissionId, const std::string& action, const std::string& remark);
    static oatpp::Object<ItemDetailedResponse> updateItem(int64_t itemId, const oatpp::Object<ItemUpdateRequest>& request);
    static oatpp::Object<StatusResultDto> updateItemStatus(int64_t itemId, const std::string& status);
};

#endif
