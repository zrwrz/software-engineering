#ifndef ORDER_CONTROLLER_HPP
#define ORDER_CONTROLLER_HPP

#include "dto/request/AuditRequest.hpp"
#include "dto/request/ReturnOrderRequest.hpp"
#include "dto/response/Response.hpp"
#include "service/OrderManageService.hpp"
#include "utils/AuthHelper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

/**
 * 预约借还管理控制器
 * 参考文档：前后端接口文档 6 预约借还接口；需求报告 预约审核、确认借出、确认归还。
 */
class OrderController : public oatpp::web::server::api::ApiController {
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
    OrderController(const std::shared_ptr<ObjectMapper>& objectMapper, const std::string& secret)
        : oatpp::web::server::api::ApiController(objectMapper), secret(secret) {}

    static std::shared_ptr<OrderController> createShared(const std::shared_ptr<ObjectMapper>& objectMapper, const std::string& secret) {
        return std::make_shared<OrderController>(objectMapper, secret);
    }

    /**
     * @brief 订单列表（管理员）
     */
    ENDPOINT("GET", "/api/v1/orders", listOrders,
        HEADER(String, authorization, "Authorization"), QUERY(Int32, page, "page"), QUERY(Int32, size, "size"), QUERY(String, status, "status"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(OrderManageService::listOrders(page ? page.getValue(1) : 1, size ? size.getValue(20) : 20, status ? status->c_str() : ""));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_403, 1003, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 订单详情
     */
    ENDPOINT("GET", "/api/v1/orders/{orderId}", getOrderDetail,
        HEADER(String, authorization, "Authorization"), PATH(Int64, orderId, "orderId"))
    {
        try {
            AuthHelper::parseToken(secret, authorization);
            return createSuccessResponse(OrderManageService::getOrderDetail(orderId));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_404, 2001, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 审核预约（管理员）
     */
    ENDPOINT("PATCH", "/api/v1/orders/{orderId}/audit", auditOrder,
        HEADER(String, authorization, "Authorization"), PATH(Int64, orderId, "orderId"), BODY_DTO(oatpp::Object<AuditRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(OrderManageService::auditOrder(context.userId, orderId, request));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_400, 2002, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 办理借出（管理员）
     */
    ENDPOINT("PATCH", "/api/v1/orders/{orderId}/borrow", borrowOrder,
        HEADER(String, authorization, "Authorization"), PATH(Int64, orderId, "orderId"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(OrderManageService::borrowOrder(context.userId, orderId));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_400, 2002, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 办理归还（管理员）
     */
    ENDPOINT("PATCH", "/api/v1/orders/{orderId}/return", returnOrder,
        HEADER(String, authorization, "Authorization"), PATH(Int64, orderId, "orderId"), BODY_DTO(oatpp::Object<ReturnOrderRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(OrderManageService::returnOrder(context.userId, orderId, request));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_400, 2002, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 逾期任务（系统任务）
     */
    ENDPOINT("POST", "/api/v1/internal/orders/mark-overdue", markOverdue)
    {
        try { return createSuccessResponse(OrderManageService::markOverdue()); }
        catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_400, 2002, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif
