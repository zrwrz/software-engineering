#ifndef LOGIN_RESPONSE_HPP
#define LOGIN_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"
#include "dto/entity/UserDto.hpp"  // 复用用户信息 DTO

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
    DTO_FIELD(Object<UserDto>, userInfo);
};

#include OATPP_CODEGEN_END(DTO)

#endif