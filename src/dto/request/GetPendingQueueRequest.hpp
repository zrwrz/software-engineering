#ifndef GET_PENDING_QUEUE_REQUEST_HPP
#define GET_PENDING_QUEUE_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 获取待审核队列的请求参数
 */
class GetPendingQueueRequest : public oatpp::DTO {
    DTO_INIT(GetPendingQueueRequest, DTO)

    DTO_FIELD(Int64, page) = 1;   // 当前页码（默认从第 1 页开始）
    DTO_FIELD(Int64, size) = 10;  // 每页条数（默认 10 条）
    DTO_FIELD(Int64, userId);     // 可选：按提交人 ID 筛选
    DTO_FIELD(String, keyword);   // 可选：按物品名称模糊搜索
};

#include OATPP_CODEGEN_END(DTO)

#endif