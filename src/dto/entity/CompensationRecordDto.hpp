#ifndef COMPENSATION_RECORD_DTO_HPP
#define COMPENSATION_RECORD_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 赔偿记录 DTO
 */
class CompensationRecordDto : public oatpp::DTO {
    DTO_INIT(CompensationRecordDto, DTO)

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
