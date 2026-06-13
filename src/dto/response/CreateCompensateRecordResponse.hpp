#ifndef CREATE_COMPENSATE_RECORD_RESPONSE_HPP
#define CREATE_COMPENSATE_RECORD_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class CreateCompensateRecordResponse : public oatpp::DTO {
    DTO_INIT(CreateCompensateRecordResponse, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(Int64, orderId);
    DTO_FIELD(Int64, userId);
    DTO_FIELD(Float64, amount);
    DTO_FIELD(String, reason);
    DTO_FIELD(String, status);
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif