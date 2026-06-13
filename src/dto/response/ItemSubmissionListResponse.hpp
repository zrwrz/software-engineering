#ifndef ITEM_SUBMISSION_LIST_RESPONSE_HPP
#define ITEM_SUBMISSION_LIST_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"
#include "dto/entity/ItemSubmissionDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 物品上传审核队列响应 DTO
 */
class ItemSubmissionListResponse : public oatpp::DTO {
    DTO_INIT(ItemSubmissionListResponse, DTO)

    DTO_FIELD(Vector<Object<ItemSubmissionDto>>, list);
    DTO_FIELD(Int32, page);
    DTO_FIELD(Int32, size);
    DTO_FIELD(Int64, total);
};

#include OATPP_CODEGEN_END(DTO)

#endif
