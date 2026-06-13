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
#include "dto/response/ChangePasswordSuccessDto.hpp"
#include "dto/response/Response.hpp"

#include "utils/jwt.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class AuthController : public oatpp::web::server::api::ApiController {
  private:
    const std::string secret;
    std::shared_ptr<UserService> user_service;

    /**
	 * @brief 构建统一格式的错误响应（私有辅助方法）
	 * @param status HTTP 状态码
	 * @param code 业务错误码
	 * @param message 错误提示信息
	 * @return 统一格式的 HTTP 响应
	 */
	std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> createErrorResponse(
		Status status, Int32 code, const oatpp::String& message) 
	{
		auto response = ApiResponse::createShared();
		response->code = code;
		response->message = message;
		return createDtoResponse(status, response);
	}
	
	/**
	 * @brief 构建统一格式的正常响应（私有辅助方法）
	 * @param status HTTP 状态码
	 * @param data 返回体
	 * @return 统一格式的 HTTP 响应
	 */
	std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> createOKResponse(
		Status status, const oatpp::Any& data) 
	{
		auto response = ApiResponse::createShared();
		response->code = 0;
		response->message = "ok";
		response->data = data;
		return createDtoResponse(status, response);
	}


  public:
    AuthController(
        const std::shared_ptr<ObjectMapper>& objectMapper,
        const std::string &secret,
        const std::shared_ptr<UserService>& user_service = nullptr
        )
        :
        oatpp::web::server::api::ApiController(objectMapper),
        secret(secret),
        user_service(user_service)
    {}

    static std::shared_ptr<AuthController> createShared(
        const std::shared_ptr<ObjectMapper>& objectMapper,
        const std::string &secret,
        const std::shared_ptr<UserService>& userService = nullptr) {
        return std::make_shared<AuthController>(objectMapper, secret, userService);
    }

	/**
	 * @brief 用户注册
	 */
	ENDPOINT("POST", "/api/v1/auth/register", registerUser,
		BODY_DTO(oatpp::Object<RegisterRequest>, request))
	{
		// 1. 参数校验
		if (!request || !request->username || request->username->empty()) {
			return createErrorResponse(Status::CODE_400, 1001, "用户名不能为空");
		}
		if (!request->password || request->password->empty()) {
			return createErrorResponse(Status::CODE_400, 1001, "密码不能为空");
		}
		if (!request->phone || request->phone->empty()) {
			return createErrorResponse(Status::CODE_400, 1001, "手机号不能为空");
		}

		// 2. 调用业务逻辑层
		try {
			auto response = user_service->registerUser(request);
			return createOKResponse(Status::CODE_201, response);
		} catch (const std::runtime_error& e) {
			return createErrorResponse(Status::CODE_401, 4001, e.what());
		} catch (const std::exception& e) {
			return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
		}
	}

	/**
	 * @brief 用户登录
	 */
	ENDPOINT("POST", "/api/v1/auth/login", login,
		BODY_DTO(oatpp::Object<LoginRequest>, request))
	{
		// 1. 参数校验
		if (!request || !request->username || request->username->empty()) {
			return createErrorResponse(Status::CODE_400, 1002, "用户名不能为空");
		}
		if (!request->password || request->password->empty()) {
			return createErrorResponse(Status::CODE_400, 1002, "密码不能为空");
		}

		// 2. 调用业务逻辑层
		try {
			auto response = user_service->login(secret, request);
			return createOKResponse(Status::CODE_200, response);
		} catch (const std::runtime_error& e) {
			// 捕获业务层抛出的特定业务异常（如：密码错误、账号不存在）
			return createErrorResponse(Status::CODE_401, 4001, e.what());
		} catch (const std::exception& e) {
			// 捕获其他未知异常
			return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
		}
	}

    /** 
	 * @brief 获取当前登录用户的详细信息
	 * 
	 * 处理获取当前用户信息的请求：
	 * 1. 从请求头中提取 Authorization 字段（JWT Token）
	 * 2. 解析 Token 获取当前用户身份
	 * 3. 查询并组装用户详细信息
	 * 4. 返回统一格式的用户信息响应
	 * 
	 */
	ENDPOINT("GET", "/api/v1/users/me", getCurrentUser,
		HEADER(String, authorization, "Authorization")) 
	{
		// 1. 校验 Authorization 头并提取 Token
		const char* BEARER_PREFIX = "Bearer ";
		const std::size_t PREFIX_LEN = 7;

		// 使用 find 方法替代 startsWith，避免 IntelliSense 报错
		if (!authorization || authorization->find(BEARER_PREFIX) != 0) {
			return createErrorResponse(
				Status::CODE_401, 
				1002, 
				"非法的 Header 格式"
			);
		}

		// 2. 截取 "Bearer " 后面的内容，即为真正的 JWT Token
		oatpp::String token = authorization->substr(PREFIX_LEN);

        // 2. 调用 JWT 验证函数
		nlohmann::json payload;
		try {
			payload = JWT::verify(secret, std::string(token->c_str()));
		} catch (const std::runtime_error& e) {
			// 捕获 JWT 验证抛出的异常（如：Token expired, Invalid signature）
			return createErrorResponse(Status::CODE_401, 1002, e.what());
		}

		int64_t userId = payload.value("userId", 0);
        
        // 3. 调用业务逻辑层获取用户信息（添加 try-catch 保护）
        try {
            auto userProfile = user_service->getUserDetailed(userId);
            return createOKResponse(Status::CODE_200, userProfile);
        } catch (const std::runtime_error& e) {
            // 业务异常（如：用户不存在）
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
            // 数据库操作失败或其他异常
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
	}

    /**
	 * @brief 修改当前登录用户的个人资料
	 */
	ENDPOINT("PUT", "/api/v1/users/me", updateProfile,
		HEADER(String, authorization, "Authorization"),
		BODY_DTO(oatpp::Object<UpdateProfileRequest>, request))
    {
		// 1. 校验 Authorization 头并提取 Token（复用您之前的逻辑）
		const char* BEARER_PREFIX = "Bearer ";
		if (!authorization || authorization->find(BEARER_PREFIX) != 0) {
			return createErrorResponse(Status::CODE_401, 1002, "非法的 Header 格式");
		}
		oatpp::String token = authorization->substr(7);

		// 2. 验证 JWT 并获取 userId
		nlohmann::json payload;
		try {
			payload = JWT::verify(secret, std::string(token->c_str()));
		} catch (const std::runtime_error& e) {
			return createErrorResponse(Status::CODE_401, 1002, e.what());
		}
		int64_t userId = payload.value("userId", int64_t(0));

		// 3. 参数校验
		if (!request) {
			return createErrorResponse(Status::CODE_400, 1001, "请求体不能为空");
		}

		// 4. 调用业务逻辑层更新资料
		try {
			auto updatedUser = user_service->updateProfile(userId, request);
			return createOKResponse(Status::CODE_200, updatedUser);
		} catch (const std::runtime_error& e) {
			// 业务异常（如：用户不存在、手机号已被占用等）
			return createErrorResponse(Status::CODE_400, 4001, e.what());
		} catch (const std::exception& e) {
			// 数据库操作失败或其他系统异常
			return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
		}
	}

    /**
	 * @brief 修改当前登录用户的密码
	 */
	ENDPOINT("PUT", "/api/v1/users/me/password", changePassword,
		HEADER(String, authorization, "Authorization"),
		BODY_DTO(oatpp::Object<ChangePasswordRequest>, request))
    {
		// 1. 校验 Authorization 头并提取 Token
		const char* BEARER_PREFIX = "Bearer ";
		if (!authorization || authorization->find(BEARER_PREFIX) != 0) {
			return createErrorResponse(Status::CODE_401, 1002, "非法的 Header 格式");
		}
		oatpp::String token = authorization->substr(7);

		// 2. 验证 JWT 并获取 userId
		nlohmann::json payload;
		try {
			payload = JWT::verify(secret, std::string(token->c_str()));
		} catch (const std::runtime_error& e) {
			return createErrorResponse(Status::CODE_401, 1002, e.what());
		}
		int64_t userId = payload.value("userId", int64_t(0));

		// 3. 调用业务逻辑层修改密码
		try {
			bool success = user_service->changePassword(userId, request);
			auto success_dto = SuccessDto::createShared();
            success_dto->success = success;

			return createOKResponse(Status::CODE_200, success_dto);
		} catch (const std::runtime_error& e) {
			return createErrorResponse(Status::CODE_401, 4001, e.what());
		} catch (const std::exception& e) {
			// 捕获其他未知异常
			return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
		}
	}
};

#include OATPP_CODEGEN_END(ApiController)

#endif