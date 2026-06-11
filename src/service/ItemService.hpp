#ifndef ITEM_SERVICE_HPP
#define ITEM_SERVICE_HPP

#include "dto/request/ItemListRequest.hpp"
#include "dto/response/ItemListResponse.hpp"

#include "dto/response/ItemDetailedResponse.hpp"

#include "dto/request/CreateItemRequest.hpp"
#include "dto/response/CreateItemResponse.hpp"

#include "dto/request/GetPendingQueueRequest.hpp"
#include "dto/response/GetPendingQueueResponse.hpp"

#include "dto/request/AuditSubmissionRequest.hpp"
#include "dto/response/AuditSubmissionResponse.hpp"

#include "dto/request/UpdateItemRequest.hpp"
#include "dto/response/UpdateItemResponse.hpp"

#include "dto/request/UpdateItemStatusRequest.hpp"
#include "dto/response/UpdateItemStatusResponse.hpp"

class ItemService {
  public:
    /**
     * @brief 筛选数据库中已注册物品
     * 
     * 根据传入的页码、页大小、关键字、类别和上架状态查询数据库中对应物品。
     * 其中关键字(keyword)采用模糊查找，类别(category)和状态(status)为精确查找。
     * 可选字段(keyword、category、status)未传入时对应对象为 nullptr。
     * 
     * @param request 查询请求对象，包含分页参数及可选的筛选条件
     * @return oatpp::Object<ItemListResponse> 分页物品列表响应对象，包含以下字段：
     *         - list: 物品对象列表，每个对象包含物品ID、名称、分类、当前可用数量和状态
     *         - page: 当前页码
     *         - size: 每页条数
     *         - total: 符合条件的总记录数
     */
    static oatpp::Object<ItemListResponse> selectItemList(
        const oatpp::Object<ItemListRequest>& request
    );

    /**
     * @brief 通过给定的物品查询具体物品信息
     * @param id 指定的物品 id
     * @return oatpp::Object<ItemDetailedResponse> 物品详细信息返回体
     */
    static oatpp::Object<ItemDetailedResponse> getItemDetailed(
        int64_t id
    );

    /**
     * @brief 提交物品上传申请（进入待审核队列）
     * @param userId 提交人用户ID（从 JWT 中解析）
     * @param request 包含物品名称、分类、描述、总数、押金等
     * @return 返回申请ID和初始状态(PENDING)
     */
    oatpp::Object<CreateItemResponse> registerItem(
        int64_t userId, 
        const oatpp::Object<CreateItemRequest>& request
    );

    /**
     * @brief 分页查询物品上传审核队列
     * @param request 包含分页参数及可选的 status 筛选（默认 PENDING）
     * @return 返回审核队列列表及分页信息
     */
    oatpp::Object<GetPendingQueueResponse> getAuditItemList(
        const oatpp::Object<GetPendingQueueRequest>& request
    );

    /**
     * @brief 审核物品上传申请（通过则自动创建物品，拒绝则更新状态）
     * @param submissionId 申请记录ID
     * @param request 包含审核动作(APPROVE/REJECT)及备注
     * @return 返回更新后的申请状态（若通过则同时返回生成的物品ID）
     */
    oatpp::Object<AuditSubmissionResponse> auditSubmission(
        int64_t submissionId, 
        const oatpp::Object<AuditSubmissionRequest>& request
    );

    /**
     * @brief 编辑物品信息（管理员按需更新）
     * @param itemId 物品ID
     * @param request 包含需要更新的字段（name, category, description, totalCount, deposit）
     * @return 返回更新后的完整物品信息
     */
    oatpp::Object<UpdateItemResponse> updateItemInfo(
        int64_t itemId, 
        const oatpp::Object<UpdateItemRequest>& request
    );

    /**
     * @brief 切换物品上下架状态
     * @param itemId 物品ID
     * @param request 包含目标状态(ON_SHELF / OFF_SHELF)
     * @return 返回物品ID及更新后的状态
     */
    oatpp::Object<UpdateItemStatusResponse> updateItemStatus(
        int64_t itemId, 
        const oatpp::Object<UpdateItemStatusRequest>& request
    );
};

#endif