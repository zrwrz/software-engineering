#ifndef UPDATE_CREDIT_RESPONSE_HPP
#define UPDATE_CREDIT_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class UpdateCreditResponse : public oatpp::DTO {
    DTO_INIT(UpdateCreditResponse, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(Int64, userId);
    DTO_FIELD(Int32, changeValue);
    DTO_FIELD(String, reason);
    DTO_FIELD(Int32, currentCreditScore);
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif