#ifndef AUDIT_RESERVATION_REQUEST_HPP
#define AUDIT_RESERVATION_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 审核预约请求参数
 */
class AuditReservationRequest : public oatpp::DTO {
    DTO_INIT(AuditReservationRequest, DTO)

    DTO_FIELD(String, action);
    DTO_FIELD(String, reason) = nullptr;
};

#include OATPP_CODEGEN_END(DTO)

#endif