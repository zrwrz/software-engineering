#ifndef ITEM_SUBMISSION_DTO_HPP
#define ITEM_SUBMISSION_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 物品上传申请 DTO
 */
class ItemSubmissionDto : public oatpp::DTO {
    DTO_INIT(ItemSubmissionDto, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(Int64, submitterId);
    DTO_FIELD(String, name);
    DTO_FIELD(String, category);
    DTO_FIELD(String, description);
    DTO_FIELD(Int32, totalCount);
    DTO_FIELD(Float64, deposit);
    DTO_FIELD(String, auditRemark);
    DTO_FIELD(String, status);
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif
