#ifndef CHANGE_PASSWORD_REQUEST_HPP
#define CHANGE_PASSWORD_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 修改密码请求
 */
class ChangePasswordRequest : public oatpp::DTO {
    DTO_INIT(ChangePasswordRequest, DTO)

    DTO_FIELD(String, oldPassword);
    DTO_FIELD(String, newPassword);
};

#include OATPP_CODEGEN_END(DTO)

#endif