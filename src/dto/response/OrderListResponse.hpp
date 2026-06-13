#ifndef ORDER_LIST_RESPONSE_HPP
#define ORDER_LIST_RESPONSE_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "dto/entity/OrderDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 管理端订单列表响应 DTO
 */
class OrderListResponse : public oatpp::DTO {
    DTO_INIT(OrderListResponse, DTO)

    DTO_FIELD(Vector<Object<OrderDto>>, list);
    DTO_FIELD(Int32, page);
    DTO_FIELD(Int32, size);
    DTO_FIELD(Int64, total);
};

#include OATPP_CODEGEN_END(DTO)

#endif
