#ifndef TOP_BORROW_ITEM_INFO_DTO_HPP
#define TOP_BORROW_ITEM_INFO_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class TopBorrowItemInfoDto : public oatpp::DTO {
    DTO_INIT(TopBorrowItemInfoDto, DTO)

    DTO_FIELD(Int64, itemId);
    DTO_FIELD(String, name);
    DTO_FIELD(Int64, count);
};

#include OATPP_CODEGEN_END(DTO)

#endif