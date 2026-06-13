#ifndef CREATE_RESERVATION_REQUEST_HPP
#define CREATE_RESERVATION_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 创建预约请求体
 */
class CreateReservationRequest : public oatpp::DTO {
    DTO_INIT(CreateReservationRequest, DTO)

    DTO_FIELD(Int64, itemId);
    DTO_FIELD(Int32, quantity);
    DTO_FIELD(String, reserveStartTime);
    DTO_FIELD(String, reserveEndTime);
    DTO_FIELD(String, remark) = nullptr;
};

#include OATPP_CODEGEN_END(DTO)

#endif