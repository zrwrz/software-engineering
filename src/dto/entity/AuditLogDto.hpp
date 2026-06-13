#ifndef AUDIT_LOG_DTO_HPP
#define AUDIT_LOG_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 审计日志 DTO
 */
class AuditLogDto : public oatpp::DTO {
    DTO_INIT(AuditLogDto, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(Int64, operatorId);
    DTO_FIELD(String, action);
    DTO_FIELD(String, target);
    DTO_FIELD(String, detail);
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif
