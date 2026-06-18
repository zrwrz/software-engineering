#ifndef ITEM_LIST_RESPONSE_HPP
#define ITEM_LIST_RESPONSE_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "dto/entity/ItemListItemDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ItemListResponse : public oatpp::DTO {
    DTO_INIT(ItemListResponse, DTO)

    DTO_FIELD(Vector<Object<ItemListItemDto>>, list);
    DTO_FIELD(Int32, total);
    DTO_FIELD(Int32, page);
    DTO_FIELD(Int32, size);
};

#include OATPP_CODEGEN_END(DTO)

#endif