#ifndef LOGIN_USERINFO_RESPONSE_DTO_HPP
#define LOGIN_USERINFO_RESPONSE_DTO_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

// 1. 开始 DTO 代码生成块
#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 登录响应中的用户信息 DTO
 * 仅包含前端展示所需的安全字段，隔离了底层数据库实体的敏感信息。
 */
class LoginUserInfoResponseDto : public oatpp::DTO {
    DTO_INIT(LoginUserInfoResponseDto, DTO)

    DTO_FIELD(Int64, id, "id");             // 用户ID
    DTO_FIELD(String, username, "username"); // 用户名
    DTO_FIELD(String, role, "role");         // 用户角色
    DTO_FIELD(Int32, creditScore, "creditScore"); // 信用分
};

// 2. 结束 DTO 代码生成块
#include OATPP_CODEGEN_END(DTO)

#endif // LOGINUSERINFORESPONSEDTO_HPP