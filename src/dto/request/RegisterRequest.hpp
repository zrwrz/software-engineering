#ifndef REGISTER_REQUEST_HPP
#define REGISTER_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 提交注册
 * 前端发送用户名、密码、电话、邮箱
 */
class RegisterRequest : public oatpp::DTO {
    DTO_INIT(RegisterRequest, DTO)

    DTO_FIELD(String, username);      // 用户名
    DTO_FIELD(String, password); 
    DTO_FIELD(String, phone);
    DTO_FIELD(String, email);
};

#include OATPP_CODEGEN_END(DTO)

#endif