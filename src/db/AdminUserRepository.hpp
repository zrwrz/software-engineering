#ifndef ADMIN_USER_REPOSITORY_HPP
#define ADMIN_USER_REPOSITORY_HPP

#include "dto/entity/StatusResultDto.hpp"
#include "dto/response/AdminUserListResponse.hpp"
#include <string>

/**
 * 管理端用户数据访问层
 */
class AdminUserRepository {
  public:
    static oatpp::Object<AdminUserListResponse> listUsers(int32_t page, int32_t size, const std::string& keyword, const std::string& status);
    static oatpp::Object<StatusResultDto> updateUserStatus(int64_t userId, const std::string& status);
};

#endif
