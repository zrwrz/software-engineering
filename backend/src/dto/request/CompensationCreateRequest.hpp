#ifndef COMPENSATION_CREATE_REQUEST_HPP
#define COMPENSATION_CREATE_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 新增赔偿记录请求 DTO
 */
class CompensationCreateRequest : public oatpp::DTO {
    DTO_INIT(CompensationCreateRequest, DTO)

    DTO_FIELD(Int64, orderId);
    DTO_FIELD(Int64, userId);
    DTO_FIELD(Float64, amount);
    DTO_FIELD(String, reason);
};

#include OATPP_CODEGEN_END(DTO)

#endif
