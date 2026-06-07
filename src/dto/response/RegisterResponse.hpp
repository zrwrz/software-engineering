#ifndef REGISTER_RESPONSE_HPP
#define REGISTER_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 注册响应：返回用户ID
 */
class RegisterResponse : public oatpp::DTO {
    DTO_INIT(RegisterResponse, DTO)

    DTO_FIELD(Int64, userId);
};

#include OATPP_CODEGEN_END(DTO)

#endif