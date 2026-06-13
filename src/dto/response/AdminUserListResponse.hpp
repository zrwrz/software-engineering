#ifndef ADMIN_USER_LIST_RESPONSE_HPP
#define ADMIN_USER_LIST_RESPONSE_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "dto/entity/AdminUserListItemDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 管理端用户列表响应 DTO
 */
class AdminUserListResponse : public oatpp::DTO {
    DTO_INIT(AdminUserListResponse, DTO)

    DTO_FIELD(Vector<Object<AdminUserListItemDto>>, list);
    DTO_FIELD(Int32, page);
    DTO_FIELD(Int32, size);
    DTO_FIELD(Int64, total);
};

#include OATPP_CODEGEN_END(DTO)

#endif
