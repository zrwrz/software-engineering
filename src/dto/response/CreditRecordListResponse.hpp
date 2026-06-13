#ifndef CREDIT_RECORD_LIST_RESPONSE_HPP
#define CREDIT_RECORD_LIST_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"
#include "dto/entity/CreditRecordDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 信用记录列表响应 DTO
 */
class CreditRecordListResponse : public oatpp::DTO {
    DTO_INIT(CreditRecordListResponse, DTO)

    DTO_FIELD(Vector<Object<CreditRecordDto>>, list);
    DTO_FIELD(Int32, page);
    DTO_FIELD(Int32, size);
    DTO_FIELD(Int64, total);
};

#include OATPP_CODEGEN_END(DTO)

#endif
