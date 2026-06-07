#ifndef UPDATE_PROFILE_REQUEST_HPP
#define UPDATE_PROFILE_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 修改个人资料请求
 */
class UpdateProfileRequest : public oatpp::DTO {
    DTO_INIT(UpdateProfileRequest, DTO)

    DTO_FIELD(String, phone);
    DTO_FIELD(String, email);
};

#include OATPP_CODEGEN_END(DTO)

#endif