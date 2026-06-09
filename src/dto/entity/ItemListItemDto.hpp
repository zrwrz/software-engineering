#ifndef ITEM_LIST_ITEM_DTO_HPP
#define ITEM_LIST_ITEM_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

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
    DTO_FIELD(Int32, availableCount);
    DTO_FIELD(String, status);
};

#include OATPP_CODEGEN_END(DTO)

#endif