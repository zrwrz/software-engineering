#ifndef REGISTER_REQUEST_HPP
#define REGISTER_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 请求盐值（注册第一步）
 * 前端发送用户名、手机号、邮箱，后端返回盐值
 */
class RegisterSaltRequest : public oatpp::DTO {
    DTO_INIT(RegisterSaltRequest, DTO)

    DTO_FIELD(String, username);
    DTO_FIELD(String, phone);
    DTO_FIELD(String, email);
};

/**
 * 提交注册（注册第二步）
 * 前端发送用户名、盐值、密码哈希
 */
class RegisterCommitRequest : public oatpp::DTO {
    DTO_INIT(RegisterCommitRequest, DTO)

    DTO_FIELD(String, username);      // 用户名（与第一步一致）
    DTO_FIELD(String, salt);          // 第一步返回的盐值
    DTO_FIELD(String, passwordHash);  // 用盐值加密后的密码哈希
};

#include OATPP_CODEGEN_END(DTO)

#endif