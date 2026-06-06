#ifndef UPDATE_ITEM_STATUS_REQUEST_HPP
#define UPDATE_ITEM_STATUS_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 物品状态更新请求
 * 用于所有物品状态变更操作
 */
class UpdateItemStatusRequest : public oatpp::DTO {
    DTO_INIT(UpdateItemStatusRequest, DTO)

    /**
     * 目标状态
     * - pending: 提交审核
     * - available: 上架（仅管理员）
     * - off_shelf: 下架
     * - maintenance: 挂起/维护中
     */
    DTO_FIELD(String, targetStatus, "targetStatus");
    
    /**
     * 操作原因/备注（可选）
     * 例如：下架原因、挂起原因等
     */
    DTO_FIELD(String, reason, "reason");
};

#include OATPP_CODEGEN_END(DTO)

#endif