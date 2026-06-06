#ifndef CREATE_ORDER_REQUEST_HPP
#define CREATE_ORDER_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 创建预约订单请求
 * 用户提交借用申请
 */
class CreateOrderRequest : public oatpp::DTO {
    DTO_INIT(CreateOrderRequest, DTO)

    DTO_FIELD(Int64, itemId, "itemId");               // 物品ID
    DTO_FIELD(String, startTime, "startTime");        // 借用开始时间 yyyy-MM-dd HH:mm:ss
    DTO_FIELD(String, endTime, "endTime");            // 借用结束时间 yyyy-MM-dd HH:mm:ss
    DTO_FIELD(String, purpose, "purpose");            // 借用用途说明
};

#include OATPP_CODEGEN_END(DTO)

#endif