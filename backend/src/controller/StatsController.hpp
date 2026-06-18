#ifndef STATS_CONTROLLER_HPP
#define STATS_CONTROLLER_HPP

#include "dto/response/Response.hpp"
#include "service/StatisticsService.hpp"
#include "utils/AuthHelper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

/**
 * 统计看板控制器
 */
class StatsController : public oatpp::web::server::api::ApiController {
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
    StatsController(const std::shared_ptr<ObjectMapper>& objectMapper, const std::string& secret)
        : oatpp::web::server::api::ApiController(objectMapper), secret(secret) {}

    static std::shared_ptr<StatsController> createShared(const std::shared_ptr<ObjectMapper>& objectMapper, const std::string& secret) {
        return std::make_shared<StatsController>(objectMapper, secret);
    }

    /**
     * @brief 统计看板
     */
    ENDPOINT("GET", "/api/v1/stats/dashboard", getDashboard,
        HEADER(String, authorization, "Authorization"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(StatisticsService::getDashboard());
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_403, 1003, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif
