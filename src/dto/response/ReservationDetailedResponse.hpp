#ifndef RESERVATION_DETAILED_RESPONSE_HPP
#define RESERVATION_DETAILED_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 预约详情完整信息对象
 */
class ReservationDetailedResponse : public oatpp::DTO {
    DTO_INIT(ReservationDetailedResponse, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(String, orderNo);
    DTO_FIELD(Int64, userId);
    DTO_FIELD(Int64, itemId);
    DTO_FIELD(Int32, quantity);
    DTO_FIELD(String, reserveStartTime);
    DTO_FIELD(String, reserveEndTime);
    DTO_FIELD(String, pickupTime) = nullptr;
    DTO_FIELD(String, returnTime) = nullptr;
    DTO_FIELD(String, status);
    DTO_FIELD(String, remark) = nullptr;
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif