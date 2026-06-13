#ifndef COMPENSATE_RECORD_REQUEST_HPP
#define COMPENSATE_RECORD_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class CompensateRecordRequest : public oatpp::DTO {
    DTO_INIT(CompensateRecordRequest, DTO)

    DTO_FIELD(Int32, page);
    DTO_FIELD(Int32, size);
    DTO_FIELD(String, status);
    DTO_FIELD(Int64, userId);
    DTO_FIELD(Int64, orderId);
};

#include OATPP_CODEGEN_END(DTO)

#endif