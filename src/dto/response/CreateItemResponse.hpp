#ifndef CREATE_ITEM_RESPONSE_HPP
#define CREATE_ITEM_RESPONSE_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 创建物品响应
 * 后端处理成功后，返回申请创建记录的 ID
 */
class CreateItemResponse : public oatpp::DTO {
    DTO_INIT(CreateItemResponse, DTO)

    DTO_FIELD(Int64, submissionId);                // 后端生成的提交记录 ID
    DTO_FIELD(String, status);
};

#include OATPP_CODEGEN_END(DTO)

#endif