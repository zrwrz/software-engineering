#ifndef ITEM_DTO_HPP
#define ITEM_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 物品信息 DTO（响应）
 */
class ItemDto : public oatpp::DTO {
    DTO_INIT(ItemDto, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(String, name);
    DTO_FIELD(String, category);
    DTO_FIELD(String, description);
    DTO_FIELD(String, imageUrl);
    DTO_FIELD(String, status);
    DTO_FIELD(Int64, createdBy);
    DTO_FIELD(String, createdAt);

    DTO_FIELD(String, updatedAt);

    DTO_FIELD(String, creatorName);
};

#include OATPP_CODEGEN_END(DTO)

#endif