#ifndef ITEM_SERVICE_HPP
#define ITEM_SERVICE_HPP

#include "dto/request/AuditSubmissionRequest.hpp"
#include "dto/request/CreateItemRequest.hpp"
#include "dto/request/ItemListRequest.hpp"
#include "dto/request/UpdateItemRequest.hpp"
#include "dto/request/UpdateItemStatusRequest.hpp"
#include "dto/response/AuditSubmissionResponse.hpp"
#include "dto/response/CreateItemResponse.hpp"
#include "dto/response/ItemDetailedResponse.hpp"
#include "dto/response/ItemListResponse.hpp"
#include "dto/response/ItemSubmissionListResponse.hpp"
#include "dto/response/UpdateItemResponse.hpp"
#include "dto/response/UpdateItemStatusResponse.hpp"

#include <string>

/**
 * 物品平台业务服务
 */
class ItemService {
  public:
    /**
     * @brief 筛选数据库中已注册物品
     */
    static oatpp::Object<ItemListResponse> selectItemList(const oatpp::Object<ItemListRequest>& request);

    /**
     * @brief 筛选数据库中已注册物品
     */
    static oatpp::Object<ItemListResponse> selectItemList(int32_t page, int32_t size, const std::string& keyword, const std::string& category, const std::string& status);

    /**
     * @brief 通过给定的物品查询具体物品信息
     */
    static oatpp::Object<ItemDetailedResponse> getItemDetailed(int64_t id);

    /**
     * @brief 提交物品上传申请（进入待审核队列）
     */
    static oatpp::Object<CreateItemResponse> registerItem(int64_t userId, const oatpp::Object<CreateItemRequest>& request);

    /**
     * @brief 分页查询物品上传审核队列
     */
    static oatpp::Object<ItemSubmissionListResponse> getAuditItemList(int32_t page, int32_t size, const std::string& status);

    /**
     * @brief 审核物品上传申请（通过则自动创建物品，拒绝则更新状态）
     */
    static oatpp::Object<AuditSubmissionResponse> auditSubmission(int64_t operatorId, int64_t submissionId, const oatpp::Object<AuditSubmissionRequest>& request);

    /**
     * @brief 编辑物品信息（管理员按需更新）
     */
    static oatpp::Object<UpdateItemResponse> updateItemInfo(int64_t operatorId, int64_t itemId, const oatpp::Object<UpdateItemRequest>& request);

    /**
     * @brief 切换物品上下架状态
     */
    static oatpp::Object<UpdateItemStatusResponse> updateItemStatus(int64_t operatorId, int64_t itemId, const oatpp::Object<UpdateItemStatusRequest>& request);
};

#endif
