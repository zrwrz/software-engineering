#ifndef CREDIT_RECORD_DTO_HPP
#define CREDIT_RECORD_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 信用记录 DTO
 */
class CreditRecordDto : public oatpp::DTO {
    DTO_INIT(CreditRecordDto, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(Int64, userId);
    DTO_FIELD(Int64, orderId);
    DTO_FIELD(Int32, changeValue);
    DTO_FIELD(String, reason);
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif
