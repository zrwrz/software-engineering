#ifndef COMPENSATE_RECORD_RESPONSE_HPP
#define COMPENSATE_RECORD_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"
#include "dto/entity/CompensateListItemDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class CompensateRecordResponse : public oatpp::DTO {
    DTO_INIT(CompensateRecordResponse, DTO)

    DTO_FIELD(Vector<Object<CompensateListItemDto>>, list);
    DTO_FIELD(Int32, page);
    DTO_FIELD(Int32, size);
    DTO_FIELD(Int64, total);
};

#include OATPP_CODEGEN_END(DTO)

#endif