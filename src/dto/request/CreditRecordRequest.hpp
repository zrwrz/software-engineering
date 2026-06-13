#ifndef CREDIT_RECORD_REQUEST_HPP
#define CREDIT_RECORD_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class CreditRecordRequest : public oatpp::DTO {
    DTO_INIT(CreditRecordRequest, DTO)

    DTO_FIELD(Int64, userId);
    DTO_FIELD(Int32, page);
    DTO_FIELD(Int32, size);
};

#include OATPP_CODEGEN_END(DTO)

#endif