#ifndef STATUS_RESULT_DTO_HPP
#define STATUS_RESULT_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 通用状态结果 DTO
 */
class StatusResultDto : public oatpp::DTO {
    DTO_INIT(StatusResultDto, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(String, status);
    DTO_FIELD(String, remark);
    DTO_FIELD(Int64, itemId);
    DTO_FIELD(String, pickupTime);
    DTO_FIELD(String, returnTime);
    DTO_FIELD(Boolean, needCompensation);
};

#include OATPP_CODEGEN_END(DTO)

#endif
