#ifndef AUDIT_SUBMISSION_RESPONSE_HPP
#define AUDIT_SUBMISSION_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 审核物品上传申请响应
 * 返回审核后的申请状态及关联的物品ID（若通过）
 */
class AuditSubmissionResponse : public oatpp::DTO {
    DTO_INIT(AuditSubmissionResponse, DTO)

    DTO_FIELD(Int64, submissionId);      // 申请ID
    DTO_FIELD(String, status);           // 更新后的状态："APPROVED" 或 "REJECTED"
    DTO_FIELD(Int64, itemId);            // 物品ID（仅在 APPROVE 时有值，REJECT 时可为空或0）
};

#include OATPP_CODEGEN_END(DTO)

#endif