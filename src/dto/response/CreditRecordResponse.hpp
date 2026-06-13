#ifndef CREDIT_RECORD_RESPONSE_HPP
#define CREDIT_RECORD_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"
#include "dto/entity/CreditListItemDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class CreditRecordResponse : public oatpp::DTO {
    DTO_INIT(CreditRecordResponse, DTO)

    DTO_FIELD(Vector<Object<CreditListItemDto>>, list);
    DTO_FIELD(Int32, page);
    DTO_FIELD(Int32, size);
    DTO_FIELD(Int64, total);
};

#include OATPP_CODEGEN_END(DTO)

#endif