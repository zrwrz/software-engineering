#ifndef UPDATE_ORDER_STATUS_REQUEST_HPP
#define UPDATE_ORDER_STATUS_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 订单状态更新请求
 * 管理员审核、确认借出等操作
 */
class UpdateOrderStatusRequest : public oatpp::DTO {
    DTO_INIT(UpdateOrderStatusRequest, DTO)

    /**
     * 目标状态
     * - APPROVED: 审核通过
     * - REJECTED: 审核拒绝
     * - BORROWED: 确认借出
     * - RETURNED: 确认归还
     * - CANCELLED: 取消订单（用户）
     */
    DTO_FIELD(String, action, "action");
    
    /**
     * 操作备注/原因
     * 审核拒绝时必填，其他可选
     */
    DTO_FIELD(String, remark, "remark");
};

#include OATPP_CODEGEN_END(DTO)

#endif