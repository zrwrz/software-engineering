#ifndef UPDATE_ITEM_RESPONSE_HPP
#define UPDATE_ITEM_RESPONSE_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 编辑物品响应
 * 返回更新后的完整物品信息
 */
class UpdateItemResponse : public oatpp::DTO {
    DTO_INIT(UpdateItemResponse, DTO)

    DTO_FIELD(Int64, id);                // 物品 ID
    DTO_FIELD(String, name);             // 物品名称
    DTO_FIELD(String, category);         // 物品分类
    DTO_FIELD(String, description);      // 物品描述
    DTO_FIELD(Int64, totalCount);        // 物品总数量
    DTO_FIELD(Int64, availableCount);    // 当前可用数量
    DTO_FIELD(String, status);           // 物品状态（如 "ON_SHELF"）
    DTO_FIELD(Float64, deposit);         // 押金金额
    DTO_FIELD(Int64, createdBy);         // 创建人 ID
    DTO_FIELD(String, createdAt);        // 创建时间
};

#include OATPP_CODEGEN_END(DTO)

#endif