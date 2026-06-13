#ifndef PENDING_QUEUE_ITEM_DTO_HPP
#define PENDING_QUEUE_ITEM_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 审核队列中的单个物品项
 */
class PendingQueueItem : public oatpp::DTO {
    DTO_INIT(PendingQueueItem, DTO)

    DTO_FIELD(Int64, id);                // 提交记录 ID
    DTO_FIELD(Int64, submitterId);       // 提交人 ID
    DTO_FIELD(String, name);             // 物品名称
    DTO_FIELD(String, category);         // 物品分类
    DTO_FIELD(String, description);      // 物品描述
    DTO_FIELD(Int32, totalCount);        // 物品总数量
    DTO_FIELD(Float64, deposit);         // 押金金额
    DTO_FIELD(String, status);           // 物品状态（如 "PENDING"）
    DTO_FIELD(String, createdAt);        // 创建时间
};


#include OATPP_CODEGEN_END(DTO)

#endif