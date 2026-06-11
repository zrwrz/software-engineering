#ifndef ITEM_CONTROLLER_HPP
#define ITEM_CONTROLLER_HPP

#include "dto/request/ItemListRequest.hpp"
#include "dto/response/ItemListResponse.hpp"

#include "dto/response/ItemDetailedResponse.hpp"

#include "dto/request/CreateItemRequest.hpp"
#include "dto/response/CreateItemResponse.hpp"

#include "dto/request/GetPendingQueueRequest.hpp"
#include "dto/response/GetPendingQueueResponse.hpp"

#include "dto/request/AuditSubmissionRequest.hpp"
#include "dto/response/AuditSubmissionResponse.hpp"

#include "dto/request/UpdateItemRequest.hpp"
#include "dto/response/UpdateItemResponse.hpp"

#include "dto/request/UpdateItemStatusRequest.hpp"
#include "dto/response/UpdateItemStatusResponse.hpp"

#include "dto/response/Response.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/codegen.hpp"

#include "service/ItemService.hpp"
#include "service/UserService.hpp"
#include "utils/jwt.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class ItemController : public oatpp::web::server::api::ApiController {
  private:
    const std::string secret;
    std::shared_ptr<ItemService> item_service;
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
    
    ItemController(
        const std::shared_ptr<ObjectMapper>& objectMapper,
        const std::string &secret,
        const std::shared_ptr<ItemService>& item_service = nullptr,
        const std::shared_ptr<UserService>& user_service = nullptr
        )
        :
        oatpp::web::server::api::ApiController(objectMapper),
        secret(secret),
        item_service(item_service),
        user_service(user_service)
    {}

    static std::shared_ptr<ItemController> createShared(
        const std::shared_ptr<ObjectMapper>& objectMapper,
        const std::string &secret,
        const std::shared_ptr<ItemService>& ItemService = nullptr,
        const std::shared_ptr<UserService>& user_service = nullptr) {
        return std::make_shared<ItemController>(objectMapper, secret, ItemService, user_service);
    }

    /**
     * @brief 获取物品列表（支持分页与多条件筛选）
     */
    ENDPOINT("GET", "api/v1/items", getItemList,
             QUERIES(Object<ItemListRequest>, queryParams))
    {
        // 1. 校验分页参数的合法性
        if (!queryParams || queryParams->page <= 0 || queryParams->size <= 0) {
            return createErrorResponse(Status::CODE_400, 1001, "分页参数错误：page 和 size 必须大于 0");
        }

        // 2. 调用 Service 层获取数据
        try {
            auto result = item_service->selectItemList(queryParams);
            
            // 3. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::exception& e) {
            // 4. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("查询物品列表失败: ") + e.what());
        }
    }

    /**
     * @brief 获取物品详细信息
     */
    ENDPOINT("GET", "api/v1/items/{itemId}", getItemDetailed,
             PATH(Int64, itemId))
    {
        // 1. 校验路径参数合法性
        if (itemId <= 0) {
            return createErrorResponse(Status::CODE_400, 1001, "无效的物品 ID");
        }

        // 2. 调用 Service 层获取详情
        try {
            auto result = item_service->getItemDetailed(itemId);
            
            // 3. 如果 Service 层返回空，说明物品不存在
            if (!result) {
                return createErrorResponse(Status::CODE_404, 2001, "物品不存在");
            }
            
            // 4. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::exception& e) {
            // 5. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("获取物品详情失败: ") + e.what());
        }
    }

    /**
     * @brief 提交物品上传申请（普通用户）
     */
    ENDPOINT("POST", "api/v1/items/submissions", submitItemApplication,
             HEADER(String, authorization, "Authorization"), // 添加 Header
             BODY_DTO(Object<CreateItemRequest>, request))
    {
        // 1. 添加鉴权逻辑
        auto userId = getUserIdFromAuth(authorization);
        if (userId == 0) {
            return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
        }
        
        // 2. 校验请求体是否为空
        if (!request) {
            return createErrorResponse(Status::CODE_400, 1001, "请求体不能为空");
        }

        // 3. 调用 Service 层处理申请提交逻辑
        try {
            auto result = item_service->registerItem(userId, request);
            
            // 4. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::exception& e) {
            // 5. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("提交物品申请失败: ") + e.what());
        }
    }

    /**
     * @brief 查询物品上传审核队列（管理员）
     */
    ENDPOINT("GET", "api/v1/admin/items/submissions", getSubmissionQueue,
             HEADER(String, authorization, "Authorization"), // 添加 Header
             QUERIES(Object<GetPendingQueueRequest>, queryParams))
    {
        // 1. 添加鉴权逻辑
        auto userId = getUserIdFromAuth(authorization);
        if (userId == 0) {
            return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
        }
        
        // 2. 校验管理员权限
        if (!user_service->isAdmin(userId)) {
            return createErrorResponse(Status::CODE_403, 1003, "权限不足：仅管理员可访问");
        }

        // 3. 调用 Service 层获取审核队列数据
        try {
            auto result = item_service->getAuditItemList(queryParams);
            
            // 4. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::exception& e) {
            // 5. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("查询审核队列失败: ") + e.what());
        }
    }

    /**
     * @brief 审核物品上传申请（管理员）
     */
    ENDPOINT("PATCH", "api/v1/admin/items/submissions/{submissionId}/audit", auditPostingApplication,
             HEADER(String, authorization, "Authorization"), // 添加 Header
             PATH(Int64, submissionId),
             BODY_DTO(Object<AuditSubmissionRequest>, request))
    {
        // 1. 添加鉴权逻辑
        auto userId = getUserIdFromAuth(authorization);
        if (userId == 0) {
            return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
        }
        
        // 2. 校验管理员权限
        if (!user_service->isAdmin(userId)) {
            return createErrorResponse(Status::CODE_403, 1003, "权限不足：仅管理员可审核");
        }

        // 3. 校验请求体及路径参数合法性
        if (!request || submissionId <= 0) {
            return createErrorResponse(Status::CODE_400, 1001, "无效的申请 ID 或请求体为空");
        }

        // 4. 调用 Service 层处理审核逻辑（内部会处理 APPROVE 自动创建物品 / REJECT 更新状态）
        try {
            auto result = item_service->auditSubmission(submissionId, request);
            
            // 5. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::exception& e) {
            // 6. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("审核物品申请失败: ") + e.what());
        }
    }

    /**
     * @brief 编辑物品信息（管理员，按需更新）
     */
    ENDPOINT("PUT", "api/v1/items/{itemId}", updateItem,
             HEADER(String, authorization, "Authorization"), // 添加 Header
             PATH(Int64, itemId),
             BODY_DTO(Object<UpdateItemRequest>, request))
    {
        // 1. 添加鉴权逻辑
        auto userId = getUserIdFromAuth(authorization);
        if (userId == 0) {
            return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
        }
        
        // 2. 校验管理员权限
        if (!user_service->isAdmin(userId)) {
            return createErrorResponse(Status::CODE_403, 1003, "权限不足：仅管理员可编辑物品");
        }

        // 3. 校验路径参数及请求体合法性
        if (itemId <= 0 || !request) {
            return createErrorResponse(Status::CODE_400, 1001, "无效的物品 ID 或请求体为空");
        }

        // 4. 调用 Service 层处理物品信息更新逻辑（内部处理按需更新字段）
        try {
            auto result = item_service->updateItemInfo(itemId, request);
            
            // 5. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::exception& e) {
            // 6. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("编辑物品信息失败: ") + e.what());
        }
    }

    /**
     * @brief 上下架物品（管理员）
     */
    ENDPOINT("PATCH", "api/v1/items/{itemId}/status", updateItemStatus,
             HEADER(String, authorization, "Authorization"), // 添加 Header
             PATH(Int64, itemId),
             BODY_DTO(Object<UpdateItemStatusRequest>, request))
    {
        // 1. 添加鉴权逻辑
        auto userId = getUserIdFromAuth(authorization);
        if (userId == 0) {
            return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
        }
        
        // 2. 校验管理员权限
        if (!user_service->isAdmin(userId)) {
            return createErrorResponse(Status::CODE_403, 1003, "权限不足：仅管理员可操作物品状态");
        }

        // 3. 校验路径参数及请求体合法性
        if (itemId <= 0 || !request) {
            return createErrorResponse(Status::CODE_400, 1001, "无效的物品 ID 或请求体为空");
        }

        // 4. 调用 Service 层处理状态更新逻辑（具体业务规则由前端控制，Service 层直接执行状态变更）
        try {
            auto result = item_service->updateItemStatus(itemId, request);
            
            // 5. 返回统一格式的成功响应
            return createOKResponse(Status::CODE_200, result);
        } catch (const std::exception& e) {
            // 6. 捕获异常并返回统一格式的错误响应
            return createErrorResponse(Status::CODE_500, 5000, std::string("更新物品状态失败: ") + e.what());
        }
    }
};

#endif