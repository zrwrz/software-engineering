#ifndef ITEM_LIST_ITEM_DTO_HPP
#define ITEM_LIST_ITEM_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 物品列表项 DTO（精简版）
 * 用于列表页展示
 */
class ItemListItemDto : public oatpp::DTO {
    DTO_INIT(ItemListItemDto, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(String, name);
    DTO_FIELD(String, category);
    DTO_FIELD(String, imageUrl);
    DTO_FIELD(String, status);        // 普通用户只看到 available
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif