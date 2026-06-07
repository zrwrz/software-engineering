#ifndef AUTH_CONTROLLER_HPP
#define AUTH_CONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/codegen.hpp"
#include "dto/request/RegisterRequest.hpp"
#include "dto/response/RegisterResponse.hpp"
#include "dto/request/LoginRequest.hpp"
#include "dto/response/LoginResponse.hpp"
#include "dto/request/UpdateProfileRequest.hpp"
#include "dto/request/ChangePasswordRequest.hpp"
#include "dto/entity/UserDto.hpp"
#include "service/UserService.hpp"

#include "utils/jwt.hpp"

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
             BODY_DTO(oatpp::Object<RegisterRequest>, request))
    {
        // 1. 参数校验：避免抛出异常，直接返回 400 Bad Request
        if (!request || !request->username || request->username->empty()) {
            return createResponse(
                Status::CODE_400, 
                "{\"code\":1001,\"message\":\"用户名不能为空\"}"
            );
        }
        if (!request->password || request->password->empty()) {
            return createResponse(
                Status::CODE_400, 
                "{\"code\":1001,\"message\":\"密码不能为空\"}"
            );
        }
        if (!request->phone || request->phone->empty()) {
            return createResponse(
                Status::CODE_400, 
                "{\"code\":1001,\"message\":\"手机号不能为空\"}"
            );
        }
        
        // 2. 调用业务逻辑层
        try {
            auto response = UserService::registerUser(request);
            return createDtoResponse(Status::CODE_201, response); // 注册成功返回 201
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
			 BODY_DTO(oatpp::Object<LoginRequest>, request))
	{
		// 1. 参数校验
		if (!request || !request->username || request->username->empty()) {
			return createResponse(
				Status::CODE_400, 
				"{\"code\":1002,\"message\":\"用户名不能为空\"}"
			);
		}
		if (!request->password || request->password->empty()) {
			return createResponse(
				Status::CODE_400, 
				"{\"code\":1002,\"message\":\"密码不能为空\"}"
			);
		}
		
		// 2. 调用业务逻辑层
		try {
			auto response = UserService::login(request);
			return createDtoResponse(Status::CODE_200, response); // 登录成功返回 200
		} catch (const std::runtime_error& e) {
			// 捕获业务层抛出的特定业务异常（如：密码错误、账号不存在）
			return createResponse(
				Status::CODE_401, 
				"{\"code\":1002,\"message\":\"" + std::string(e.what()) + "\"}"
			);
		} catch (const std::exception& e) {
			// 捕获其他未知异常
			return createResponse(
				Status::CODE_500, 
				"{\"code\":5000,\"message\":\"系统内部错误\"}"
			);
		}
	}

    /**
     * @brief 获取当前用户信息
     */
    ENDPOINT("GET", "/users/me", getCurrentUser)
    {
        // 1. 获取 Authorization 头
        auto authHeader = request->getHeader("Authorization");
        if (!authHeader || authHeader->empty()) {
            return createResponse(Status::CODE_401, "{\"code\":1002,\"message\":\"缺少Token\"}");
        }
        
        // 2. 验证 Token，获取 userId
        auto userId = authService->verifyToken(authHeader);
        if (userId <= 0) {
            return createResponse(Status::CODE_401, "{\"code\":1002,\"message\":\"Token无效或已过期\"}");
        }
        
        // 3. 查询用户信息
        auto user = userService->getUserById(userId);
        if (!user) {
            return createResponse(Status::CODE_404, "{\"code\":2001,\"message\":\"用户不存在\"}");
        }
        
        // 4. 转换为 DTO 并返回
        auto dto = user->toDto();
        return createDtoResponse(Status::CODE_200, dto);
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif