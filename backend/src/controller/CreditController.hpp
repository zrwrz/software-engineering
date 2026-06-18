#ifndef CREDIT_CONTROLLER_HPP
#define CREDIT_CONTROLLER_HPP

#include "dto/request/CreditAdjustRequest.hpp"
#include "dto/response/Response.hpp"
#include "service/CreditService.hpp"
#include "utils/AuthHelper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

/**
 * 信用积分控制器
 */
class CreditController : public oatpp::web::server::api::ApiController {
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
    CreditController(const std::shared_ptr<ObjectMapper>& objectMapper, const std::string& secret)
        : oatpp::web::server::api::ApiController(objectMapper), secret(secret) {}

    static std::shared_ptr<CreditController> createShared(const std::shared_ptr<ObjectMapper>& objectMapper, const std::string& secret) {
        return std::make_shared<CreditController>(objectMapper, secret);
    }

    /**
     * @brief 查询用户信用记录
     */
    ENDPOINT("GET", "/api/v1/credits/records", listCreditRecords,
        HEADER(String, authorization, "Authorization"), QUERY(Int64, userId, "userId"), QUERY(Int32, page, "page"), QUERY(Int32, size, "size"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            return createSuccessResponse(CreditService::listRecords(context.userId, context.role, userId ? userId.getValue(0) : 0,
                page ? page.getValue(1) : 1, size ? size.getValue(10) : 10));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_403, 1003, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 调整信用分（管理员）
     */
    ENDPOINT("POST", "/api/v1/credits/adjust", adjustCredit,
        HEADER(String, authorization, "Authorization"), BODY_DTO(oatpp::Object<CreditAdjustRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"system_admin"});
            return createSuccessResponse(CreditService::adjustCredit(context.userId, request));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_400, 1001, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif
