#ifndef AUTH_CONTROLLER_HPP
#define AUTH_CONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "dto/request/RegisterRequest.hpp"
#include "dto/response/RegisterResponse.hpp"

#include <random>
#include <string>

#include OATPP_CODEGEN_BEGIN(ApiController)

class AuthController : public oatpp::web::server::api::ApiController {
  private:
    /**
     * @brief 生成随机盐值
     * 
     * 生成一个32位的随机字符串，用于密码加盐。
     * 字符集包括大小写字母和数字。
     * 
     * @return std::string 32位随机字符串
     */
    std::string generateSalt() {
        static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::string salt;
        salt.reserve(32);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
        for (int i = 0; i < 32; ++i) {
            salt += charset[dis(gen)];
        }
        return salt;
    }

  public:
    AuthController(const std::shared_ptr<ObjectMapper>& objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {}
    
    ENDPOINT("POST", "/api/v1/auth/salt", getSalt,
             REQUEST_DTO(oatpp::Object<RegisterSaltRequest>, request))
    {
        auto response = RegisterSaltResponse::createShared();
        response->salt = "dummy_salt";
        response->expiresIn = 300;
        return createDtoResponse(Status::CODE_200, response);
    }
    
    // 第二步：提交注册
    ENDPOINT("POST", "/api/v1/auth/register", registerUser,
             REQUEST_DTO(oatpp::Object<RegisterCommitRequest>, request))
    {
        auto response = RegisterResponse::createShared();
        response->userId = 10001;
        return createDtoResponse(Status::CODE_201, response);
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif