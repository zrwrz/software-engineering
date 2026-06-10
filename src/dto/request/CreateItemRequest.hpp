#ifndef CREATE_ITEM_REQUEST_HPP
#define CREATE_ITEM_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 创建物品请求（仅限用户）
 * 前端提交物品信息，后端生成 id、createdAt 等
 */
class CreateItemRequest : public oatpp::DTO {
    DTO_INIT(CreateItemRequest, DTO)

    DTO_FIELD(String, name);             // 物品名称
    DTO_FIELD(String, category);         // 物品分类
    DTO_FIELD(String, description);      // 物品描述
    DTO_FIELD(Int32, totalCount);        // 物品总数量
    DTO_FIELD(Float64, deposit);         // 押金金额
    // DTO_FIELD(String, status);           // 物品状态（如 "ON_SHELF"）
};

#include OATPP_CODEGEN_END(DTO)

#endif