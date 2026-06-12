#ifndef RESERVATION_LIST_ITEM_DTO_HPP
#define RESERVATION_LIST_ITEM_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 预约列表项精简展示对象
 */
class ReservationListItemDto : public oatpp::DTO {
    DTO_INIT(ReservationListItemDto, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(String, orderNo);
    DTO_FIELD(Int64, userId);
    DTO_FIELD(Int64, itemId);
    DTO_FIELD(Int32, quantity);
    DTO_FIELD(String, status);
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif