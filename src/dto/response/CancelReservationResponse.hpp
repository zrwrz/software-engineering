#ifndef CANCEL_RESERVATION_RESPONSE_HPP
#define CANCEL_RESERVATION_RESPONSE_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 取消预约操作结果对象
 */
class CancelReservationResponse : public oatpp::DTO {
    DTO_INIT(CancelReservationResponse, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(String, status);
};

#include OATPP_CODEGEN_END(DTO)

#endif