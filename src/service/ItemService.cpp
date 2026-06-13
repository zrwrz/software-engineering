#include "service/ItemService.hpp"

#include "db/AuditLogRepository.hpp"
#include "db/ItemManageRepository.hpp"
#include "dto/request/AuditRequest.hpp"
#include "dto/request/ItemSubmissionRequest.hpp"
#include "dto/request/ItemUpdateRequest.hpp"
#include "dto/request/StatusUpdateRequest.hpp"

#include <stdexcept>

namespace {
std::string toStdString(const oatpp::String& value) {
    return value ? std::string(value->c_str()) : std::string();
}

void requireText(const oatpp::String& value, const std::string& message) {
    if (!value || value->empty()) {
        throw std::runtime_error(message);
    }
}
}

oatpp::Object<ItemListResponse> ItemService::selectItemList(const oatpp::Object<ItemListRequest>& request) {
    int32_t page = request && request->page ? request->page.getValue(1) : 1;
    int32_t size = request && request->size ? request->size.getValue(10) : 10;
    std::string keyword = request && request->keyword ? request->keyword->c_str() : "";
    std::string category = request && request->category ? request->category->c_str() : "";
    std::string status = request && request->status ? request->status->c_str() : "";
    return selectItemList(page, size, keyword, category, status);
}

oatpp::Object<ItemListResponse> ItemService::selectItemList(int32_t page, int32_t size, const std::string& keyword, const std::string& category, const std::string& status) {
    return ItemManageRepository::listItems(page, size, keyword, category, status);
}

oatpp::Object<ItemDetailedResponse> ItemService::getItemDetailed(int64_t id) {
    if (id <= 0) {
        throw std::runtime_error("物品ID不能为空");
    }
    return ItemManageRepository::getItemDetail(id);
}

oatpp::Object<CreateItemResponse> ItemService::registerItem(int64_t userId, const oatpp::Object<CreateItemRequest>& request) {
    if (userId <= 0) {
        throw std::runtime_error("提交人ID不能为空");
    }
    if (!request) {
        throw std::runtime_error("请求体不能为空");
    }
    requireText(request->name, "物品名称不能为空");
    if (!request->totalCount || request->totalCount.getValue(0) <= 0) {
        throw std::runtime_error("物品数量必须大于0");
    }
    if (request->deposit && request->deposit.getValue(0.0) < 0) {
        throw std::runtime_error("押金不能为负数");
    }

    auto createRequest = ItemSubmissionRequest::createShared();
    createRequest->name = request->name;
    createRequest->category = request->category;
    createRequest->description = request->description;
    createRequest->totalCount = request->totalCount;
    createRequest->deposit = request->deposit;

    auto result = ItemManageRepository::createSubmission(userId, createRequest);

    auto response = CreateItemResponse::createShared();
    response->submissionId = result->id;
    response->status = result->status;
    return response;
}

oatpp::Object<ItemSubmissionListResponse> ItemService::getAuditItemList(int32_t page, int32_t size, const std::string& status) {
    return ItemManageRepository::listSubmissions(page, size, status.empty() ? "PENDING" : status);
}

oatpp::Object<AuditSubmissionResponse> ItemService::auditSubmission(int64_t operatorId, int64_t submissionId, const oatpp::Object<AuditSubmissionRequest>& request) {
    if (operatorId <= 0) {
        throw std::runtime_error("操作人ID不能为空");
    }
    if (submissionId <= 0) {
        throw std::runtime_error("申请ID不能为空");
    }
    if (!request || !request->action) {
        throw std::runtime_error("审核动作不能为空");
    }
    std::string action = toStdString(request->action);
    if (action != "APPROVE" && action != "REJECT") {
        throw std::runtime_error("审核动作不合法");
    }
    std::string remark = request->remark ? request->remark->c_str() : "";

    auto result = ItemManageRepository::auditSubmission(operatorId, submissionId, action, remark);
    AuditLogRepository::createLog(operatorId, "ITEM_SUBMISSION_AUDIT", "submission:" + std::to_string(submissionId), "审核物品上传申请：" + action);

    auto response = AuditSubmissionResponse::createShared();
    response->submissionId = result->id;
    response->status = result->status;
    response->itemId = result->itemId;
    return response;
}

oatpp::Object<UpdateItemResponse> ItemService::updateItemInfo(int64_t operatorId, int64_t itemId, const oatpp::Object<UpdateItemRequest>& request) {
    if (operatorId <= 0) {
        throw std::runtime_error("操作人ID不能为空");
    }
    if (itemId <= 0) {
        throw std::runtime_error("物品ID不能为空");
    }
    if (!request) {
        throw std::runtime_error("请求体不能为空");
    }
    if (request->totalCount && request->totalCount.getValue(0) < 0) {
        throw std::runtime_error("总库存不能为负数");
    }
    if (request->deposit && request->deposit.getValue(0.0) < 0) {
        throw std::runtime_error("押金不能为负数");
    }

    auto updateRequest = ItemUpdateRequest::createShared();
    updateRequest->name = request->name;
    updateRequest->category = request->category;
    updateRequest->description = request->description;
    if (request->totalCount) {
        updateRequest->totalCount = static_cast<v_int32>(request->totalCount.getValue(0));
    }
    updateRequest->deposit = request->deposit;

    auto result = ItemManageRepository::updateItem(itemId, updateRequest);
    AuditLogRepository::createLog(operatorId, "ITEM_UPDATE", "item:" + std::to_string(itemId), "编辑物品信息");

    auto response = UpdateItemResponse::createShared();
    response->id = result->id;
    response->name = result->name;
    response->category = result->category;
    response->description = result->description;
    response->totalCount = result->totalCount;
    response->availableCount = result->availableCount;
    response->status = result->status;
    response->deposit = result->deposit;
    response->createdBy = result->createdBy;
    response->createdAt = result->createdAt;
    return response;
}

oatpp::Object<UpdateItemStatusResponse> ItemService::updateItemStatus(int64_t operatorId, int64_t itemId, const oatpp::Object<UpdateItemStatusRequest>& request) {
    if (operatorId <= 0) {
        throw std::runtime_error("操作人ID不能为空");
    }
    if (itemId <= 0) {
        throw std::runtime_error("物品ID不能为空");
    }
    if (!request || !request->status) {
        throw std::runtime_error("物品状态不能为空");
    }
    std::string status = toStdString(request->status);
    if (status != "ON_SHELF" && status != "OFF_SHELF") {
        throw std::runtime_error("物品状态不合法");
    }

    auto result = ItemManageRepository::updateItemStatus(itemId, status);
    AuditLogRepository::createLog(operatorId, "ITEM_STATUS_UPDATE", "item:" + std::to_string(itemId), "更新物品状态为 " + status);

    auto response = UpdateItemStatusResponse::createShared();
    response->id = result->id;
    response->status = result->status;
    return response;
}
