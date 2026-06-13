#ifndef CREATE_COMPENSATE_RECORD_REQUEST_HPP
#define CREATE_COMPENSATE_RECORD_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class CreateCompensateRecordRequest : public oatpp::DTO {
    DTO_INIT(CreateCompensateRecordRequest, DTO)

    DTO_FIELD(Int64, orderId);
    DTO_FIELD(Int64, userId);
    DTO_FIELD(Float64, amount);
    DTO_FIELD(String, reason);
};

#include OATPP_CODEGEN_END(DTO)

#endif