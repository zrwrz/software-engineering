#ifndef ITEM_SUBMISSION_REQUEST_HPP
#define ITEM_SUBMISSION_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 物品上传申请请求 DTO
 */
class ItemSubmissionRequest : public oatpp::DTO {
    DTO_INIT(ItemSubmissionRequest, DTO)

    DTO_FIELD(String, name);
    DTO_FIELD(String, category);
    DTO_FIELD(String, description);
    DTO_FIELD(Int32, totalCount);
    DTO_FIELD(Float64, deposit);
};

#include OATPP_CODEGEN_END(DTO)

#endif
