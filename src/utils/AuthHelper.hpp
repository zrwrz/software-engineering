#ifndef AUTH_HELPER_HPP
#define AUTH_HELPER_HPP

#include "oatpp/Types.hpp"
#include "utils/jwt.hpp"
#include <stdexcept>
#include <string>
#include <vector>

/**
 * 接口鉴权辅助工具
 */
class AuthHelper {
  public:
    struct CurrentUserContext {
        int64_t userId;
        std::string role;
    };

    static CurrentUserContext parseToken(const std::string& secret, const oatpp::String& authorization) {
        const char* BEARER_PREFIX = "Bearer ";
        if (!authorization || authorization->find(BEARER_PREFIX) != 0) {
            throw std::runtime_error("非法的 Header 格式");
        }
        oatpp::String token = authorization->substr(7);
        auto payload = JWT::verify(secret, std::string(token->c_str()));
        CurrentUserContext context;
        context.userId = payload.value("userId", int64_t(0));
        context.role = payload.value("role", std::string(""));
        if (context.userId <= 0 || context.role.empty()) {
            throw std::runtime_error("Token 缺少用户信息");
        }
        return context;
    }

    static void requireAnyRole(const CurrentUserContext& context, const std::vector<std::string>& roles) {
        for (const auto& role : roles) {
            if (context.role == role) {
                return;
            }
        }
        throw std::runtime_error("无权限访问");
    }
};

#endif
