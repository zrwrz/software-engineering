#ifndef ORDER_CONTROLLER_HPP
#define ORDER_CONTROLLER_HPP

#include "dto/request/AuditReservationRequest.hpp"
#include "dto/request/CreateReservationRequest.hpp"
#include "dto/request/RegisterReturnRequest.hpp"
#include "dto/response/Response.hpp"
#include "service/OrderService.hpp"
#include "utils/AuthHelper.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

/**
 * 预约借还控制器
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
     * @brief 创建预约
     */
    ENDPOINT("POST", "/api/v1/orders/reservations", createReservation,
        HEADER(String, authorization, "Authorization"),
        BODY_DTO(oatpp::Object<CreateReservationRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"borrower", "resource_admin", "system_admin"});
            return createSuccessResponse(OrderService::createReservation(context.userId, request));
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 1001, e.what());
        } catch (...) {
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 我的预约/订单列表
     */
    ENDPOINT("GET", "/api/v1/orders/my", listMyOrders,
        HEADER(String, authorization, "Authorization"),
        QUERY(Int32, page, "page"),
        QUERY(Int32, size, "size"),
        QUERY(String, status, "status"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            return createSuccessResponse(OrderService::getMyOrders(
                context.userId,
                page ? page.getValue(1) : 1,
                size ? size.getValue(10) : 10,
                status ? status->c_str() : ""
            ));
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_401, 1002, e.what());
        } catch (...) {
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 订单列表（管理员）
     */
    ENDPOINT("GET", "/api/v1/orders", listOrders,
        HEADER(String, authorization, "Authorization"),
        QUERY(Int32, page, "page"),
        QUERY(Int32, size, "size"),
        QUERY(String, status, "status"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(OrderService::getOrderList(
                page ? page.getValue(1) : 1,
                size ? size.getValue(20) : 20,
                status ? status->c_str() : ""
            ));
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_403, 1003, e.what());
        } catch (...) {
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 订单详情
     */
    ENDPOINT("GET", "/api/v1/orders/{orderId}", getOrderDetail,
        HEADER(String, authorization, "Authorization"),
        PATH(Int64, orderId, "orderId"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            auto data = OrderService::getOrderDetailed(orderId);
            if (context.role == "borrower" && data->userId.getValue(0) != context.userId) {
                return createErrorResponse(Status::CODE_403, 1003, "无权限操作该订单");
            }
            return createSuccessResponse(data);
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_404, 2001, e.what());
        } catch (...) {
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 取消预约（用户）
     */
    ENDPOINT("PATCH", "/api/v1/orders/{orderId}/cancel", cancelReservation,
        HEADER(String, authorization, "Authorization"),
        PATH(Int64, orderId, "orderId"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            return createSuccessResponse(OrderService::cancelReservation(context.userId, context.role, orderId));
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 2002, e.what());
        } catch (...) {
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 审核预约（管理员）
     */
    ENDPOINT("PATCH", "/api/v1/orders/{orderId}/audit", auditOrder,
        HEADER(String, authorization, "Authorization"),
        PATH(Int64, orderId, "orderId"),
        BODY_DTO(oatpp::Object<AuditReservationRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(OrderService::auditReservation(context.userId, orderId, request));
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 2002, e.what());
        } catch (...) {
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 办理借出（管理员）
     */
    ENDPOINT("PATCH", "/api/v1/orders/{orderId}/borrow", borrowOrder,
        HEADER(String, authorization, "Authorization"),
        PATH(Int64, orderId, "orderId"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(OrderService::registerBorrow(context.userId, orderId));
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 2002, e.what());
        } catch (...) {
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 办理归还（管理员）
     */
    ENDPOINT("PATCH", "/api/v1/orders/{orderId}/return", returnOrder,
        HEADER(String, authorization, "Authorization"),
        PATH(Int64, orderId, "orderId"),
        BODY_DTO(oatpp::Object<RegisterReturnRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(OrderService::registerReturn(context.userId, orderId, request));
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 2002, e.what());
        } catch (...) {
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 逾期任务（系统任务）
     */
    ENDPOINT("POST", "/api/v1/internal/orders/mark-overdue", markOverdue)
    {
        try {
            return createSuccessResponse(OrderService::markOverdue());
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 2002, e.what());
        } catch (...) {
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif
