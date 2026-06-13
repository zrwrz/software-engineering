#ifndef CREDIT_LIST_ITEM_DTO_HPP
#define CREDIT_LIST_ITEM_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class CreditListItemDto : public oatpp::DTO {
    DTO_INIT(CreditListItemDto, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(Int64, userId);
    DTO_FIELD(Int64, orderId);
    DTO_FIELD(Int32, changeValue);
    DTO_FIELD(String, reason);
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif