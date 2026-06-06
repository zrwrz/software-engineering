#ifndef REGISTER_RESPONSE_HPP
#define REGISTER_RESPONSE_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 注册第一步响应：返回盐值
 */
class RegisterSaltResponse : public oatpp::DTO {
    DTO_INIT(RegisterSaltResponse, DTO)

    DTO_FIELD(String, salt);
    DTO_FIELD(Int64, expiresIn);  // 有效期（秒）
};

/**
 * 注册第二步响应：返回用户ID
 */
class RegisterResponse : public oatpp::DTO {
    DTO_INIT(RegisterResponse, DTO)

    DTO_FIELD(Int64, userId);
};

#include OATPP_CODEGEN_END(DTO)

#endif