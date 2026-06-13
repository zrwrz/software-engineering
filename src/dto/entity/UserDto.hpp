#ifndef USER_DTO_HPP
#define USER_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 用户结构
 */
class UserDto : public oatpp::DTO {
    DTO_INIT(UserDto, DTO) 
    
    DTO_FIELD(Int64, id);
    DTO_FIELD(String, username);
    DTO_FIELD(String, role);
    DTO_FIELD(String, phone);
    DTO_FIELD(String, email);
    DTO_FIELD(Int32, creditScore);
    DTO_FIELD(String, status);
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif