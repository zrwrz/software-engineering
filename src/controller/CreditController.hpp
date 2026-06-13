#ifndef CREDIT_CONTROLLER_HPP
#define CREDIT_CONTROLLER_HPP

#include "dto/request/CreditRecordRequest.hpp"
#include "dto/response/CreditRecordResponse.hpp"

#include "dto/request/UpdateCreditRequest.hpp"
#include "dto/response/UpdateCreditResponse.hpp"

#include "dto/request/CompensateRecordRequest.hpp"
#include "dto/response/CompensateRecordResponse.hpp"

#include "dto/request/CreateCompensateRecordRequest.hpp"
#include "dto/response/CreateCompensateRecordResponse.hpp"

#include "dto/request/UpdateCompensateStatusRequest.hpp"
#include "dto/response/UpdateCompensateStatusResponse.hpp"

#include "dto/response/DashboardResponse.hpp"

#include "dto/response/Response.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/codegen.hpp"

#include "service/CreditService.hpp"
#include "service/UserService.hpp"
#include "utils/jwt.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class CreditController : public oatpp::web::server::api::ApiController {
  private:
    const std::string secret;
    std::shared_ptr<CreditService> credit_service;
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

    /**
     * @brief 从 Authorization Header 中提取并验证 JWT，获取用户ID
     * @param authorization 请求头中的 Authorization 字段
     * @return int64_t 解析出的用户ID，失败返回 0
     */
    inline int64_t getUserIdFromAuth(const oatpp::String& authorization) {
        const char* BEARER_PREFIX = "Bearer ";
        const std::size_t PREFIX_LEN = 7;
        
        if (!authorization || authorization->find(BEARER_PREFIX) != 0) {
            return 0;
        }
        
        oatpp::String token = authorization->substr(PREFIX_LEN);
        nlohmann::json payload;
        try {
            payload = JWT::verify(secret, std::string(token->c_str()));
            return payload.value("userId", int64_t(0));
        } catch (const std::runtime_error&) {
            return 0;
        }
    }

  public:
    
    CreditController(
        const std::shared_ptr<ObjectMapper>& objectMapper,
        const std::string &secret,
        const std::shared_ptr<CreditService>& credit_service = nullptr,
        const std::shared_ptr<UserService>& user_service = nullptr
        )
        :
        oatpp::web::server::api::ApiController(objectMapper),
        secret(secret),
        credit_service(credit_service),
        user_service(user_service)
    {}

    static std::shared_ptr<CreditController> createShared(
        const std::shared_ptr<ObjectMapper>& objectMapper,
        const std::string &secret,
        const std::shared_ptr<CreditService>& CreditService = nullptr,
        const std::shared_ptr<UserService>& user_service = nullptr) {
        return std::make_shared<CreditController>(objectMapper, secret, CreditService, user_service);
    }

    /**
     * @brief 查询用户信用记录
     * 
     * 管理员可查全部，用户默认查自己
     */
    ENDPOINT("GET", "api/v1/credits/records", getCreditRecords,
             HEADER(String, authorization, "Authorization"),
             QUERIES(Object<CreditRecordRequest>, queryParams))
    {
        // 1. 添加鉴权逻辑
        auto userId = getUserIdFromAuth(authorization);
        if (userId == 0) {
            return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
        }
        
        // 2. 校验分页参数的合法性
        if (!queryParams || queryParams->page <= 0 || queryParams->size <= 0) {
            return createErrorResponse(Status::CODE_400, 1001, "分页参数错误：page 和 size 必须大于 0");
        }

        // 3. 非管理员用户只能查自己的记录
        bool isAdmin = user_service->isAdmin(userId);
        if (!isAdmin) {
            queryParams->userId = userId;
        }

        // 4. 调用 Service 层获取数据
        try {
            auto result = credit_service->selectCreditRecords(queryParams);
            
            // 5. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
            // 6. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("查询信用记录失败: ") + e.what());
        }
    }

    /**
     * @brief 手工调整信用分（管理员）
     */
    ENDPOINT("POST", "api/v1/credits/adjust", adjustCredit,
             HEADER(String, authorization, "Authorization"),
             BODY_DTO(Object<UpdateCreditRequest>, request))
    {
        // 1. 添加鉴权逻辑
        auto userId = getUserIdFromAuth(authorization);
        if (userId == 0) {
            return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
        }
        
        // 2. 校验管理员权限
        if (!user_service->isAdmin(userId)) {
            return createErrorResponse(Status::CODE_403, 1003, "权限不足：仅管理员可调整信用分");
        }

        // 3. 校验请求体是否为空
        if (!request) {
            return createErrorResponse(Status::CODE_400, 1001, "请求体不能为空");
        }

        // 4. 调用 Service 层处理调整逻辑
        try {
            auto result = credit_service->adjustCredit(request);
            
            // 5. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
            // 6. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("调整信用分失败: ") + e.what());
        }
    }

    /**
     * @brief 查询赔偿记录（管理员）
     */
    ENDPOINT("GET", "api/v1/compensations", getCompensateRecords,
             HEADER(String, authorization, "Authorization"),
             QUERIES(Object<CompensateRecordRequest>, queryParams))
    {
        // 1. 添加鉴权逻辑
        auto userId = getUserIdFromAuth(authorization);
        if (userId == 0) {
            return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
        }
        
        // 2. 校验管理员权限
        if (!user_service->isAdmin(userId)) {
            return createErrorResponse(Status::CODE_403, 1003, "权限不足：仅管理员可查看赔偿记录");
        }

        // 3. 校验分页参数的合法性
        if (!queryParams || queryParams->page <= 0 || queryParams->size <= 0) {
            return createErrorResponse(Status::CODE_400, 1001, "分页参数错误：page 和 size 必须大于 0");
        }

        // 4. 调用 Service 层获取数据
        try {
            auto result = credit_service->selectCompensateRecords(queryParams);
            
            // 5. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
            // 6. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("查询赔偿记录失败: ") + e.what());
        }
    }

    /**
     * @brief 新增赔偿记录（管理员）
     */
    ENDPOINT("POST", "api/v1/compensations", createCompensateRecord,
             HEADER(String, authorization, "Authorization"),
             BODY_DTO(Object<CreateCompensateRecordRequest>, request))
    {
        // 1. 添加鉴权逻辑
        auto userId = getUserIdFromAuth(authorization);
        if (userId == 0) {
            return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
        }
        
        // 2. 校验管理员权限
        if (!user_service->isAdmin(userId)) {
            return createErrorResponse(Status::CODE_403, 1003, "权限不足：仅管理员可新增赔偿记录");
        }

        // 3. 校验请求体是否为空
        if (!request) {
            return createErrorResponse(Status::CODE_400, 1001, "请求体不能为空");
        }

        // 4. 调用 Service 层处理创建逻辑
        try {
            auto result = credit_service->createCompensateRecord(request);
            
            // 5. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
            // 6. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("新增赔偿记录失败: ") + e.what());
        }
    }

    /**
     * @brief 更新赔偿状态（管理员）
     */
    ENDPOINT("PATCH", "api/v1/compensations/{id}/status", updateCompensateStatus,
             HEADER(String, authorization, "Authorization"),
             PATH(Int64, id),
             BODY_DTO(Object<UpdateCompensateStatusRequest>, request))
    {
        // 1. 添加鉴权逻辑
        auto userId = getUserIdFromAuth(authorization);
        if (userId == 0) {
            return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
        }
        
        // 2. 校验管理员权限
        if (!user_service->isAdmin(userId)) {
            return createErrorResponse(Status::CODE_403, 1003, "权限不足：仅管理员可更新赔偿状态");
        }

        // 3. 校验路径参数及请求体合法性
        if (id <= 0 || !request) {
            return createErrorResponse(Status::CODE_400, 1001, "无效的记录 ID 或请求体为空");
        }

        // 4. 调用 Service 层处理状态更新逻辑
        try {
            auto result = credit_service->updateCompensateStatus(id, request);
            
            // 5. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
            // 6. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("更新赔偿状态失败: ") + e.what());
        }
    }
    
    /**
     * @brief 统计看板
     */
    ENDPOINT("GET", "api/v1/stats/dashboard", getDashboard,
             HEADER(String, authorization, "Authorization"))
    {
        // 1. 添加鉴权逻辑
        auto userId = getUserIdFromAuth(authorization);
        if (userId == 0) {
            return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
        }
        
        // 2. 校验管理员权限
        if (!user_service->isAdmin(userId)) {
            return createErrorResponse(Status::CODE_403, 1003, "权限不足：仅管理员可查看统计看板");
        }

        // 3. 调用 Service 层获取统计数据
        try {
            auto result = credit_service->getDashboard();
            
            // 4. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
            // 5. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("获取统计看板失败: ") + e.what());
        }
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif