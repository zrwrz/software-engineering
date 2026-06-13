#ifndef CREDIT_ADJUST_RESULT_DTO_HPP
#define CREDIT_ADJUST_RESULT_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 信用调整结果 DTO
 */
class CreditAdjustResultDto : public oatpp::DTO {
    DTO_INIT(CreditAdjustResultDto, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(Int64, userId);
    DTO_FIELD(Int32, changeValue);
    DTO_FIELD(String, reason);
    DTO_FIELD(Int32, currentCreditScore);
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif
