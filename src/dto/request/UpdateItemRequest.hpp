#ifndef UPDATE_ITEM_REQUEST_HPP
#define UPDATE_ITEM_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 编辑物品请求（管理员）
 * 按需传递字段，未传递的字段保持为 nullptr，后端据此判断是否需要更新
 */
class UpdateItemRequest : public oatpp::DTO {
    DTO_INIT(UpdateItemRequest, DTO)

    DTO_FIELD(String, name);             // 物品名称
    DTO_FIELD(String, category);         // 物品分类
    DTO_FIELD(String, description);      // 物品描述
    DTO_FIELD(Int64, totalCount);        // 物品总数量
    DTO_FIELD(Float64, deposit);         // 押金金额
};

#include OATPP_CODEGEN_END(DTO)

#endif