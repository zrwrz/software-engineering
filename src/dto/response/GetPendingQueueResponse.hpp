#ifndef GET_PENDING_QUEUE_RESPONSE_HPP
#define GET_PENDING_QUEUE_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"
#include "dto/entity/PendingQueueItemDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 查询物品上传审核队列响应（管理员）
 */
class GetPendingQueueResponse : public oatpp::DTO {
    DTO_INIT(GetPendingQueueResponse, DTO)

    DTO_FIELD(List<Object<PendingQueueItem>>, list); // 审核队列列表
    DTO_FIELD(Int32, page);                          // 当前页码
    DTO_FIELD(Int32, size);                          // 每页条数
    DTO_FIELD(Int32, total);                         // 总记录数
};

#include OATPP_CODEGEN_END(DTO)

#endif