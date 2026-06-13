#ifndef ITEM_CONTROLLER_HPP
#define ITEM_CONTROLLER_HPP

#include "dto/request/AuditRequest.hpp"
#include "dto/request/ItemSubmissionRequest.hpp"
#include "dto/request/ItemUpdateRequest.hpp"
#include "dto/request/StatusUpdateRequest.hpp"
#include "dto/response/Response.hpp"
#include "service/ItemManageService.hpp"
#include "utils/AuthHelper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

/**
 * 物品平台控制器
 */
class ItemController : public oatpp::web::server::api::ApiController {
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
    ItemController(const std::shared_ptr<ObjectMapper>& objectMapper, const std::string& secret)
        : oatpp::web::server::api::ApiController(objectMapper), secret(secret) {}

    static std::shared_ptr<ItemController> createShared(const std::shared_ptr<ObjectMapper>& objectMapper, const std::string& secret) {
        return std::make_shared<ItemController>(objectMapper, secret);
    }

    /**
     * @brief 物品列表（分页+筛选）
     */
    ENDPOINT("GET", "/api/v1/items", listItems,
        QUERY(Int32, page, "page"), QUERY(Int32, size, "size"),
        QUERY(String, keyword, "keyword"), QUERY(String, category, "category"), QUERY(String, status, "status"))
    {
        try {
            auto data = ItemManageService::listItems(page ? page.getValue(1) : 1, size ? size.getValue(10) : 10,
                keyword ? keyword->c_str() : "", category ? category->c_str() : "", status ? status->c_str() : "");
            return createSuccessResponse(data);
        } catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 1001, e.what());
        } catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 物品详情
     */
    ENDPOINT("GET", "/api/v1/items/{itemId}", getItemDetail, PATH(Int64, itemId, "itemId"))
    {
        try { return createSuccessResponse(ItemManageService::getItemDetail(itemId)); }
        catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_404, 2001, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 提交物品上传申请（用户）
     */
    ENDPOINT("POST", "/api/v1/items/submissions", createSubmission,
        HEADER(String, authorization, "Authorization"), BODY_DTO(oatpp::Object<ItemSubmissionRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"borrower", "resource_admin", "system_admin"});
            return createSuccessResponse(ItemManageService::createSubmission(context.userId, request));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_400, 1001, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 查询物品上传审核队列（管理员）
     */
    ENDPOINT("GET", "/api/v1/admin/items/submissions", listSubmissions,
        HEADER(String, authorization, "Authorization"), QUERY(Int32, page, "page"), QUERY(Int32, size, "size"), QUERY(String, status, "status"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(ItemManageService::listSubmissions(page ? page.getValue(1) : 1, size ? size.getValue(10) : 10, status ? status->c_str() : "PENDING"));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_403, 1003, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 审核物品上传申请（管理员）
     */
    ENDPOINT("PATCH", "/api/v1/admin/items/submissions/{submissionId}/audit", auditSubmission,
        HEADER(String, authorization, "Authorization"), PATH(Int64, submissionId, "submissionId"), BODY_DTO(oatpp::Object<AuditRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(ItemManageService::auditSubmission(context.userId, submissionId, request));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_400, 1001, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 编辑物品（管理员）
     */
    ENDPOINT("PUT", "/api/v1/items/{itemId}", updateItem,
        HEADER(String, authorization, "Authorization"), PATH(Int64, itemId, "itemId"), BODY_DTO(oatpp::Object<ItemUpdateRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(ItemManageService::updateItem(context.userId, itemId, request));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_400, 1001, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }

    /**
     * @brief 上下架物品（管理员）
     */
    ENDPOINT("PATCH", "/api/v1/items/{itemId}/status", updateItemStatus,
        HEADER(String, authorization, "Authorization"), PATH(Int64, itemId, "itemId"), BODY_DTO(oatpp::Object<StatusUpdateRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin", "system_admin"});
            return createSuccessResponse(ItemManageService::updateItemStatus(context.userId, itemId, request));
        } catch (const std::runtime_error& e) { return createErrorResponse(Status::CODE_400, 1001, e.what()); }
        catch (...) { return createErrorResponse(Status::CODE_500, 5000, "系统内部错误"); }
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif
