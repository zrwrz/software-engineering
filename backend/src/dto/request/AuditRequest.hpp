#ifndef AUDIT_REQUEST_HPP
#define AUDIT_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 审核请求 DTO
 */
class AuditRequest : public oatpp::DTO {
    DTO_INIT(AuditRequest, DTO)

    DTO_FIELD(String, action);
    DTO_FIELD(String, remark);
};

#include OATPP_CODEGEN_END(DTO)

#endif
