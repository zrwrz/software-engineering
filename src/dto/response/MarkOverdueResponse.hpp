#ifndef MARK_OVERDUE_RESPONSE_HPP
#define MARK_OVERDUE_RESPONSE_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 标记逾期任务响应体
 */
class MarkOverdueResponse : public oatpp::DTO {
    DTO_INIT(MarkOverdueResponse, DTO)

    DTO_FIELD(Int64, updatedCount); // 更新的订单数量
    DTO_FIELD(List<Int64>, orderIds); // 被更新的订单ID列表
};

#include OATPP_CODEGEN_END(DTO)

#endif