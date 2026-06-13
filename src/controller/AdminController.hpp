#ifndef ADMIN_CONTROLLER_HPP
#define ADMIN_CONTROLLER_HPP

#include "dto/request/StatusUpdateRequest.hpp"
#include "dto/response/Response.hpp"
#include "service/AdminUserService.hpp"
#include "utils/AuthHelper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

/**
 * 管理端用户与日志控制器
 */
class AdminController : public oatpp::web::server::api::ApiController {
  private:
    const std::string secret;

    template<class T>
    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> createSuccessResponse(const T& data) {
        auto response = ApiResponse::createShared();
        response->code = 0;
        response->message = "ok";
        response->data = data;
        return createDtoResponse(Status::CODE_200, response);
    }

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> createErrorResponse(Status status, Int32 code, const oatpp::String& message) {
        auto response = ApiResponse::createShared();
        response->code = code;
        response->message = message;
        return createDtoResponse(status, response);
    }

  public:
    AdminController(const std::shared_ptr<ObjectMapper>& objectMapper, const std::string& secret)
        : oatpp::web::server::api::ApiController(objectMapper), secret(secret) {}

    static std::shared_ptr<AdminController> createShared(const std::shared_ptr<ObjectMapper>& objectMapper, const std::string& secret) {
        return std::make_shared<AdminController>(objectMapper, secret);
    }

    /**
     * @brief 用户管理列表
     */
    ENDPOINT("GET", "/api/v1/admin/users", listUsers,
        HEADER(String, authorization, "Authorization"),
        QUERY(Int32, page, "page"),
        QUERY(Int32, size, "size"),
        QUERY(String, keyword, "keyword"),
        QUERY(String, status, "status"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"system_admin"});
            auto data = AdminUserService::listUsers(page ? page.getValue(1) : 1, size ? size.getValue(10) : 10,
                keyword ? keyword->c_str() : "", status ? status->c_str() : "");
            return createSuccessResponse(data);
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_403, 1003, e.what());
        } catch (...) {
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 启用/禁用用户
     */
    ENDPOINT("PATCH", "/api/v1/admin/users/{userId}/status", updateUserStatus,
        HEADER(String, authorization, "Authorization"),
        PATH(Int64, userId, "userId"),
        BODY_DTO(oatpp::Object<StatusUpdateRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"system_admin"});
            if (!request || !request->status) {
                return createErrorResponse(Status::CODE_400, 1001, "用户状态不能为空");
            }
            auto data = AdminUserService::updateUserStatus(context.userId, userId, request->status->c_str());
            return createSuccessResponse(data);
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 1001, e.what());
        } catch (...) {
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 审计日志查询
     */
    ENDPOINT("GET", "/api/v1/admin/audit-logs", listAuditLogs,
        HEADER(String, authorization, "Authorization"),
        QUERY(Int32, page, "page"),
        QUERY(Int32, size, "size"),
        QUERY(Int64, operatorId, "operatorId"),
        QUERY(String, startTime, "startTime"),
        QUERY(String, endTime, "endTime"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"system_admin"});
            auto data = AdminUserService::listAuditLogs(page ? page.getValue(1) : 1, size ? size.getValue(10) : 10,
                operatorId ? operatorId.getValue(0) : 0, startTime ? startTime->c_str() : "", endTime ? endTime->c_str() : "");
            return createSuccessResponse(data);
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_403, 1003, e.what());
        } catch (...) {
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif
