#ifndef LOGIN_REQUEST_HPP
#define LOGIN_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 登录请求
 * 前端提交用户名和密码（明文，由前端负责加密传输）
 */
class LoginRequest : public oatpp::DTO {
    DTO_INIT(LoginRequest, DTO)

    DTO_FIELD(String, username);
    DTO_FIELD(String, password);
};

#include OATPP_CODEGEN_END(DTO)

#endif