#ifndef CREATE_RESERVATION_RESPONSE_HPP
#define CREATE_RESERVATION_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 创建预约响应体
 */
class CreateReservationResponse : public oatpp::DTO {
    DTO_INIT(CreateReservationResponse, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(String, orderNo);
    DTO_FIELD(Int64, userId);
    DTO_FIELD(Int64, itemId);
    DTO_FIELD(Int32, quantity);
    DTO_FIELD(String, reserveStartTime);
    DTO_FIELD(String, reserveEndTime);
    DTO_FIELD(String, status);
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif