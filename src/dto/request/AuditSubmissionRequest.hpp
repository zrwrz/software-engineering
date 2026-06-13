#ifndef AUDIT_SUBMISSION_REQUEST_HPP
#define AUDIT_SUBMISSION_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 审核物品上传申请请求（管理员）
 * action: APPROVE / REJECT
 */
class AuditSubmissionRequest : public oatpp::DTO {
    DTO_INIT(AuditSubmissionRequest, DTO)

    DTO_FIELD(String, action);           // 审核动作："APPROVE" 或 "REJECT"
    DTO_FIELD(String, remark);           // 审核备注/拒绝原因
};

#include OATPP_CODEGEN_END(DTO)

#endif