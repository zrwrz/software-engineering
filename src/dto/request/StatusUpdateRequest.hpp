#ifndef STATUS_UPDATE_REQUEST_HPP
#define STATUS_UPDATE_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 状态更新请求 DTO
 */
class StatusUpdateRequest : public oatpp::DTO {
    DTO_INIT(StatusUpdateRequest, DTO)

    DTO_FIELD(String, status);
};

#include OATPP_CODEGEN_END(DTO)

#endif
