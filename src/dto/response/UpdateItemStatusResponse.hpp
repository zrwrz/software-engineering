#ifndef UPDATE_ITEM_STATUS_RESPONSE_HPP
#define UPDATE_ITEM_STATUS_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 上下架物品响应
 * 返回更新后的物品 ID 及最新状态
 */
class UpdateItemStatusResponse : public oatpp::DTO {
    DTO_INIT(UpdateItemStatusResponse, DTO)

    DTO_FIELD(Int64, id);                // 物品 ID
    DTO_FIELD(String, status);           // 更新后的物品状态
};

#include OATPP_CODEGEN_END(DTO)

#endif