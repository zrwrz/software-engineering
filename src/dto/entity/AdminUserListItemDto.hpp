#ifndef ADMIN_USER_LIST_ITEM_DTO_HPP
#define ADMIN_USER_LIST_ITEM_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 管理端用户列表项 DTO
 */
class AdminUserListItemDto : public oatpp::DTO {
    DTO_INIT(AdminUserListItemDto, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(String, username);
    DTO_FIELD(String, role);
    DTO_FIELD(String, phone);
    DTO_FIELD(Int32, creditScore);
    DTO_FIELD(String, status);
    DTO_FIELD(String, createdAt);
};

#include OATPP_CODEGEN_END(DTO)

#endif
