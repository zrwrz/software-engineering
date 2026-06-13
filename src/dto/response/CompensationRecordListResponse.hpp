#ifndef COMPENSATION_RECORD_LIST_RESPONSE_HPP
#define COMPENSATION_RECORD_LIST_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"
#include "dto/entity/CompensationRecordDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 赔偿记录列表响应 DTO
 */
class CompensationRecordListResponse : public oatpp::DTO {
    DTO_INIT(CompensationRecordListResponse, DTO)

    DTO_FIELD(Vector<Object<CompensationRecordDto>>, list);
    DTO_FIELD(Int32, page);
    DTO_FIELD(Int32, size);
    DTO_FIELD(Int64, total);
};

#include OATPP_CODEGEN_END(DTO)

#endif
