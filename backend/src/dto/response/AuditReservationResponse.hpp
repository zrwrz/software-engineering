#ifndef AUDIT_RESERVATION_RESPONSE_HPP
#define AUDIT_RESERVATION_RESPONSE_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 审核预约操作结果对象
 */
class AuditReservationResponse : public oatpp::DTO {
    DTO_INIT(AuditReservationResponse, DTO)
    
    DTO_FIELD(Int64, id); 
    DTO_FIELD(String, status); 
    DTO_FIELD(String, remark); 
};

#include OATPP_CODEGEN_END(DTO)

#endif