#ifndef TOP_BORROW_ITEM_DTO_HPP
#define TOP_BORROW_ITEM_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 热门借用物品 DTO
 */
class TopBorrowItemDto : public oatpp::DTO {
    DTO_INIT(TopBorrowItemDto, DTO)

    DTO_FIELD(Int64, itemId);
    DTO_FIELD(String, name);
    DTO_FIELD(Int32, count);
};

#include OATPP_CODEGEN_END(DTO)

#endif
