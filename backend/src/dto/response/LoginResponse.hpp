#ifndef LOGIN_RESPONSE_HPP
#define LOGIN_RESPONSE_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "dto/response/LoginUserInfoResponseDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 登录响应
 * 返回 JWT Token 和用户基本信息
 */
class LoginResponse : public oatpp::DTO {
    DTO_INIT(LoginResponse, DTO)

    DTO_FIELD(String, token);
    DTO_FIELD(String, tokenType) = "Bearer";
    DTO_FIELD(Int64, expiresIn);
    DTO_FIELD(Object<LoginUserInfoResponseDto>, userInfo);
};

#include OATPP_CODEGEN_END(DTO)

#endif