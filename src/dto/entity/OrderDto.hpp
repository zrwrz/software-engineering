#ifndef ORDER_DTO_HPP
#define ORDER_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 订单 DTO
 * 返回订单完整信息
 */
class OrderDto : public oatpp::DTO {
    DTO_INIT(OrderDto, DTO)

    DTO_FIELD(Int64, id, "id");
    DTO_FIELD(String, orderNo, "orderNo");            // 订单编号
    DTO_FIELD(Int64, userId, "userId");               // 借用用户ID
    DTO_FIELD(Int64, itemId, "itemId");               // 物品ID
    DTO_FIELD(String, itemName, "itemName");          // 物品名称（联表查询）

    DTO_FIELD(Int32, quantity, "quantity");           // 借用数量（默认1）

    DTO_FIELD(String, startTime, "startTime");        // 预计借用开始时间
    DTO_FIELD(String, endTime, "endTime");            // 预计借用结束时间
    DTO_FIELD(String, pickupTime, "pickupTime");      // 实际借出时间
    DTO_FIELD(String, returnTime, "returnTime");      // 实际归还时间
    
    DTO_FIELD(String, purpose, "purpose");            // 借用备注

    DTO_FIELD(String, status, "status");              // 订单状态

    DTO_FIELD(String, remark, "remark");              // 审核备注

    DTO_FIELD(Int64, createdBy, "createdBy");         // 创建人（借用用户）
    DTO_FIELD(String, createdAt, "createdAt");
    DTO_FIELD(String, updatedAt, "updatedAt");
};

#include OATPP_CODEGEN_END(DTO)

#endif