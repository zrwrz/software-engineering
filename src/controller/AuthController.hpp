#ifndef AUTH_CONTROLLER_HPP
#define AUTH_CONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "dto/request/RegisterRequest.hpp"
#include "dto/response/RegisterResponse.hpp"
#include "dto/request/LoginRequest.hpp"
#include "dto/response/LoginResponse.hpp"
#include "service/UserService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class AuthController : public oatpp::web::server::api::ApiController {
  public:
    AuthController(const std::shared_ptr<ObjectMapper>& objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {}
    
    /**
     * @brief 用户注册
     */
    ENDPOINT("POST", "/auth/register", registerUser,
             REQUEST_DTO(oatpp::Object<RegisterRequest>, request))
    {
        try {
            if (!request->username || request->username->empty()) {
                throw std::runtime_error("用户名不能为空");
            }
            if (!request->password || request->password->empty()) {
                throw std::runtime_error("密码不能为空");
            }
            if (!request->phone || request->phone->empty()) {
                throw std::runtime_error("手机号不能为空");
            }
            
            auto response = UserService::registerUser(request);
            return createDtoResponse(Status::CODE_201, response);
            
        } catch (const std::runtime_error& e) {
            return createResponse(
                Status::CODE_400,
                "{\"code\":1001,\"message\":\"" + std::string(e.what()) + "\"}"
            );
        } catch (const std::exception& e) {
            return createResponse(
                Status::CODE_500,
                "{\"code\":5000,\"message\":\"系统内部错误\"}"
            );
        }
    }
    
    /**
     * @brief 用户登录
     */
    ENDPOINT("POST", "/auth/login", login,
             REQUEST_DTO(oatpp::Object<LoginRequest>, request))
    {
        try {
            if (!request->username || request->username->empty()) {
                throw std::runtime_error("用户名不能为空");
            }
            if (!request->password || request->password->empty()) {
                throw std::runtime_error("密码不能为空");
            }
            
            auto response = UserService::login(request);
            return createDtoResponse(Status::CODE_200, response);
            
        } catch (const std::runtime_error& e) {
            return createResponse(
                Status::CODE_401,
                "{\"code\":1002,\"message\":\"" + std::string(e.what()) + "\"}"
            );
        } catch (const std::exception& e) {
            return createResponse(
                Status::CODE_500,
                "{\"code\":5000,\"message\":\"系统内部错误\"}"
            );
        }
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif