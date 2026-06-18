#ifndef CREDIT_ADJUST_REQUEST_HPP
#define CREDIT_ADJUST_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 调整信用分请求 DTO
 */
class CreditAdjustRequest : public oatpp::DTO {
    DTO_INIT(CreditAdjustRequest, DTO)

    DTO_FIELD(Int64, userId);
    DTO_FIELD(Int32, changeValue);
    DTO_FIELD(String, reason);
};

#include OATPP_CODEGEN_END(DTO)

#endif
