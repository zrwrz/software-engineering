#ifndef RESERVATION_LIST_RESPONSE_HPP
#define RESERVATION_LIST_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"
#include "dto/entity/ReservationListItemDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 预约列表分页响应体
 */
class ReservationListResponse : public oatpp::DTO {
    DTO_INIT(ReservationListResponse, DTO)

    DTO_FIELD(List<Object<ReservationListItemDto>>, list);
    DTO_FIELD(Int32, page);
    DTO_FIELD(Int32, size);
    DTO_FIELD(Int64, total);
};

#include OATPP_CODEGEN_END(DTO)

#endif