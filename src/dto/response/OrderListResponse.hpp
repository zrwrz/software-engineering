#ifndef ORDER_LIST_RESPONSE_HPP
#define ORDER_LIST_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"
#include "dto/entity/OrderDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 订单列表响应（分页）
 */
class OrderListResponse : public oatpp::DTO {
    DTO_INIT(OrderListResponse, DTO)

    DTO_FIELD(Vector<Object<OrderDto>>, list, "list");
    DTO_FIELD(Int64, total, "total");
    DTO_FIELD(Int32, page, "page");
    DTO_FIELD(Int32, size, "size");
};

#include OATPP_CODEGEN_END(DTO)

#endif