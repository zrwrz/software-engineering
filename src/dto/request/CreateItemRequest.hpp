#ifndef CREATE_ITEM_REQUEST_HPP
#define CREATE_ITEM_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 创建物品请求（管理员/登记人）
 * 前端提交物品信息，后端生成 id、createdAt 等
 */
class CreateItemRequest : public oatpp::DTO {
    DTO_INIT(CreateItemRequest, DTO)

    DTO_FIELD(String, name);
    DTO_FIELD(String, category);
    DTO_FIELD(String, description);
    DTO_FIELD(String, imageUrl);
};

#include OATPP_CODEGEN_END(DTO)

#endif