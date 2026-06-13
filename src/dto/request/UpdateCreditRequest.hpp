#ifndef UPDATE_CREDIT_REQUEST_HPP
#define UPDATE_CREDIT_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class UpdateCreditRequest : public oatpp::DTO {
    DTO_INIT(UpdateCreditRequest, DTO)

    DTO_FIELD(Int64, userId);
    DTO_FIELD(Int32, changeValue);
    DTO_FIELD(String, reason);
};

#include OATPP_CODEGEN_END(DTO)

#endif