#ifndef UPDATE_ITEM_STATUS_REQUEST_HPP
#define UPDATE_ITEM_STATUS_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 上下架物品请求（管理员）
 * 仅包含需要更新的状态字段
 */
class UpdateItemStatusRequest : public oatpp::DTO {
    DTO_INIT(UpdateItemStatusRequest, DTO)

    DTO_FIELD(String, status);           // 物品状态（如 "ON_SHELF" / "OFF_SHELF"）
};

#include OATPP_CODEGEN_END(DTO)

#endif