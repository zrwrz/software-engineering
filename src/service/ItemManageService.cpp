#include "service/ItemManageService.hpp"
#include "db/AuditLogRepository.hpp"
#include "db/ItemManageRepository.hpp"

#include <stdexcept>

oatpp::Object<ItemListResponse> ItemManageService::listItems(int32_t page, int32_t size, const std::string& keyword, const std::string& category, const std::string& status) {
    return ItemManageRepository::listItems(page, size, keyword, category, status);
}

oatpp::Object<ItemDetailedResponse> ItemManageService::getItemDetail(int64_t itemId) {
    if (itemId <= 0) throw std::runtime_error("物品ID不能为空");
    return ItemManageRepository::getItemDetail(itemId);
}

oatpp::Object<StatusResultDto> ItemManageService::createSubmission(int64_t submitterId, const oatpp::Object<ItemSubmissionRequest>& request) {
    if (!request || !request->name || request->name->empty()) throw std::runtime_error("物品名称不能为空");
    if (!request->totalCount || request->totalCount.getValue(0) <= 0) throw std::runtime_error("物品数量必须大于0");
    return ItemManageRepository::createSubmission(submitterId, request);
}

oatpp::Object<ItemSubmissionListResponse> ItemManageService::listSubmissions(int32_t page, int32_t size, const std::string& status) {
    return ItemManageRepository::listSubmissions(page, size, status.empty() ? "PENDING" : status);
}

oatpp::Object<StatusResultDto> ItemManageService::auditSubmission(int64_t operatorId, int64_t submissionId, const oatpp::Object<AuditRequest>& request) {
    if (!request || !request->action) throw std::runtime_error("审核动作不能为空");
    std::string action = request->action->c_str();
    if (action != "APPROVE" && action != "REJECT") throw std::runtime_error("审核动作不合法");
    std::string remark = request->remark ? request->remark->c_str() : "";
    auto result = ItemManageRepository::auditSubmission(operatorId, submissionId, action, remark);
    AuditLogRepository::createLog(operatorId, "ITEM_SUBMISSION_AUDIT", "submission:" + std::to_string(submissionId), "审核物品上传申请：" + action);
    return result;
}

oatpp::Object<ItemDetailedResponse> ItemManageService::updateItem(int64_t operatorId, int64_t itemId, const oatpp::Object<ItemUpdateRequest>& request) {
    if (itemId <= 0) throw std::runtime_error("物品ID不能为空");
    auto result = ItemManageRepository::updateItem(itemId, request);
    AuditLogRepository::createLog(operatorId, "ITEM_UPDATE", "item:" + std::to_string(itemId), "编辑物品信息");
    return result;
}

oatpp::Object<StatusResultDto> ItemManageService::updateItemStatus(int64_t operatorId, int64_t itemId, const oatpp::Object<StatusUpdateRequest>& request) {
    if (!request || !request->status) throw std::runtime_error("物品状态不能为空");
    std::string status = request->status->c_str();
    if (status != "ON_SHELF" && status != "OFF_SHELF") throw std::runtime_error("物品状态不合法");
    auto result = ItemManageRepository::updateItemStatus(itemId, status);
    AuditLogRepository::createLog(operatorId, "ITEM_STATUS_UPDATE", "item:" + std::to_string(itemId), "更新物品状态为 " + status);
    return result;
}
