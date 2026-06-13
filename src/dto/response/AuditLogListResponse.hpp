#ifndef AUDIT_LOG_LIST_RESPONSE_HPP
#define AUDIT_LOG_LIST_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"
#include "dto/entity/AuditLogDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 审计日志列表响应 DTO
 */
class AuditLogListResponse : public oatpp::DTO {
    DTO_INIT(AuditLogListResponse, DTO)

    DTO_FIELD(Vector<Object<AuditLogDto>>, list);
    DTO_FIELD(Int32, page);
    DTO_FIELD(Int32, size);
    DTO_FIELD(Int64, total);
};

#include OATPP_CODEGEN_END(DTO)

#endif
