#ifndef COMPENSATION_CONTROLLER_HPP
#define COMPENSATION_CONTROLLER_HPP

#include "dto/request/CompensationCreateRequest.hpp"
#include "dto/request/StatusUpdateRequest.hpp"
#include "dto/response/Response.hpp"
#include "service/CompensationService.hpp"
#include "utils/AuthHelper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

/**
 * 赔偿处理控制器
 */
class CompensationController : public oatpp::web::server::api::ApiController {
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
    CompensationController(const std::shared_ptr<ObjectMapper>& objectMapper, const std::string& secret)
        : oatpp::web::server::api::ApiController(objectMapper), secret(secret) {}

    static std::shared_ptr<CompensationController> createShared(const std::shared_ptr<ObjectMapper>& objectMapper, const std::string& secret) {
        return std::make_shared<CompensationController>(objectMapper, secret);
    }

    /**
     * @brief 查询赔偿记录（管理员）
     */
    ENDPOINT("GET", "/api/v1/compensations", listCompensations,
        HEADER(String, authorization, "Authorization"), QUERY(Int32, page, "page"), QUERY(Int32, size, "size"),
        QUERY(String, status, "status"), QUERY(Int64, userId, "userId"), QUERY(Int64, orderId, "orderId"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(CompensationService::listRecords(page ? page.getValue(1) : 1, size ? size.getValue(20) : 20,
                status ? status->c_str() : "", userId ? userId.getValue(0) : 0, orderId ? orderId.getValue(0) : 0));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_403, 1003, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 新增赔偿记录（管理员）
     */
    ENDPOINT("POST", "/api/v1/compensations", createCompensation,
        HEADER(String, authorization, "Authorization"), BODY_DTO(oatpp::Object<CompensationCreateRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(CompensationService::createRecord(context.userId, request));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_400, 1001, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 更新赔偿状态（管理员）
     */
    ENDPOINT("PATCH", "/api/v1/compensations/{id}/status", updateCompensationStatus,
        HEADER(String, authorization, "Authorization"), PATH(Int64, id, "id"), BODY_DTO(oatpp::Object<StatusUpdateRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(CompensationService::updateStatus(context.userId, id, request));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_400, 1001, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif
