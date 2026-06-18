#ifndef ITEM_CONTROLLER_HPP
#define ITEM_CONTROLLER_HPP

#include "dto/request/AuditSubmissionRequest.hpp"
#include "dto/request/CreateItemRequest.hpp"
#include "dto/request/UpdateItemRequest.hpp"
#include "dto/request/UpdateItemStatusRequest.hpp"
#include "dto/response/Response.hpp"
#include "service/ItemService.hpp"
#include "utils/AuthHelper.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/base/Environment.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

/**
 * 物品平台控制器
 */
class ItemController : public oatpp::web::server::api::ApiController {
  private:
    const std::string secret;

    void logSuccess(const char* endpoint, const std::string& detail) const {
        OATPP_LOGI("ItemController", "[OK] %s - %s", endpoint, detail.c_str());
    }

    void logFailure(const char* endpoint, const std::string& detail) const {
        OATPP_LOGW("ItemController", "[FAIL] %s - %s", endpoint, detail.c_str());
    }

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
        QUERY(Int32, page, "page"),
        QUERY(Int32, size, "size"),
        QUERY(String, keyword, "keyword"),
        QUERY(String, category, "category"),
        QUERY(String, status, "status"))
    {
        try {
            auto data = ItemService::selectItemList(
                page ? page.getValue(1) : 1,
                size ? size.getValue(10) : 10,
                keyword ? keyword->c_str() : "",
                category ? category->c_str() : "",
                status ? status->c_str() : ""
            );
            logSuccess("GET /api/v1/items", "query list success");
            return createSuccessResponse(data);
        } catch (const std::runtime_error& e) {
            logFailure("GET /api/v1/items", e.what());
            return createErrorResponse(Status::CODE_400, 1001, e.what());
        } catch (...) {
            logFailure("GET /api/v1/items", "unexpected error");
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 物品详情
     */
    ENDPOINT("GET", "/api/v1/items/{itemId}", getItemDetail,
        PATH(Int64, itemId, "itemId"))
    {
        try {
            auto data = ItemService::getItemDetailed(itemId);
            logSuccess("GET /api/v1/items/{itemId}", "itemId=" + std::to_string(itemId));
            return createSuccessResponse(data);
        } catch (const std::runtime_error& e) {
            logFailure("GET /api/v1/items/{itemId}", e.what());
            return createErrorResponse(Status::CODE_404, 2001, e.what());
        } catch (...) {
            logFailure("GET /api/v1/items/{itemId}", "unexpected error");
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 提交物品上传申请（用户）
     */
    ENDPOINT("POST", "/api/v1/items/submissions", createSubmission,
        HEADER(String, authorization, "Authorization"),
        BODY_DTO(oatpp::Object<CreateItemRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"borrower", "resource_admin", "system_admin"});
            auto data = ItemService::registerItem(context.userId, request);
            logSuccess("POST /api/v1/items/submissions",
                "userId=" + std::to_string(context.userId));
            return createSuccessResponse(data);
        } catch (const std::runtime_error& e) {
            logFailure("POST /api/v1/items/submissions", e.what());
            return createErrorResponse(Status::CODE_400, 1001, e.what());
        } catch (...) {
            logFailure("POST /api/v1/items/submissions", "unexpected error");
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 查询物品上传审核队列（管理员）
     */
    ENDPOINT("GET", "/api/v1/admin/items/submissions", listSubmissions,
        HEADER(String, authorization, "Authorization"),
        QUERY(Int32, page, "page"),
        QUERY(Int32, size, "size"),
        QUERY(String, status, "status"))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin"});
            auto data = ItemService::getAuditItemList(
                page ? page.getValue(1) : 1,
                size ? size.getValue(10) : 10,
                status ? status->c_str() : "PENDING"
            );
            logSuccess("GET /api/v1/admin/items/submissions",
                "adminUserId=" + std::to_string(context.userId));
            return createSuccessResponse(data);
        } catch (const std::runtime_error& e) {
            logFailure("GET /api/v1/admin/items/submissions", e.what());
            return createErrorResponse(Status::CODE_403, 1003, e.what());
        } catch (...) {
            logFailure("GET /api/v1/admin/items/submissions", "unexpected error");
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 审核物品上传申请（管理员）
     */
    ENDPOINT("PATCH", "/api/v1/admin/items/submissions/{submissionId}/audit", auditSubmission,
        HEADER(String, authorization, "Authorization"),
        PATH(Int64, submissionId, "submissionId"),
        BODY_DTO(oatpp::Object<AuditSubmissionRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin"});
            auto data = ItemService::auditSubmission(context.userId, submissionId, request);
            logSuccess("PATCH /api/v1/admin/items/submissions/{submissionId}/audit",
                "adminUserId=" + std::to_string(context.userId) + ", submissionId=" + std::to_string(submissionId));
            return createSuccessResponse(data);
        } catch (const std::runtime_error& e) {
            logFailure("PATCH /api/v1/admin/items/submissions/{submissionId}/audit", e.what());
            return createErrorResponse(Status::CODE_400, 1001, e.what());
        } catch (...) {
            logFailure("PATCH /api/v1/admin/items/submissions/{submissionId}/audit", "unexpected error");
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 编辑物品（管理员）
     */
    ENDPOINT("PUT", "/api/v1/items/{itemId}", updateItem,
        HEADER(String, authorization, "Authorization"),
        PATH(Int64, itemId, "itemId"),
        BODY_DTO(oatpp::Object<UpdateItemRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin"});
            auto data = ItemService::updateItemInfo(context.userId, itemId, request);
            logSuccess("PUT /api/v1/items/{itemId}",
                "adminUserId=" + std::to_string(context.userId) + ", itemId=" + std::to_string(itemId));
            return createSuccessResponse(data);
        } catch (const std::runtime_error& e) {
            logFailure("PUT /api/v1/items/{itemId}", e.what());
            return createErrorResponse(Status::CODE_400, 1001, e.what());
        } catch (...) {
            logFailure("PUT /api/v1/items/{itemId}", "unexpected error");
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }

    /**
     * @brief 上下架物品（管理员）
     */
    ENDPOINT("PATCH", "/api/v1/items/{itemId}/status", updateItemStatus,
        HEADER(String, authorization, "Authorization"),
        PATH(Int64, itemId, "itemId"),
        BODY_DTO(oatpp::Object<UpdateItemStatusRequest>, request))
    {
        try {
            auto context = AuthHelper::parseToken(secret, authorization);
            AuthHelper::requireAnyRole(context, {"resource_admin"});
            auto data = ItemService::updateItemStatus(context.userId, itemId, request);
            logSuccess("PATCH /api/v1/items/{itemId}/status",
                "adminUserId=" + std::to_string(context.userId) + ", itemId=" + std::to_string(itemId));
            return createSuccessResponse(data);
        } catch (const std::runtime_error& e) {
            logFailure("PATCH /api/v1/items/{itemId}/status", e.what());
            return createErrorResponse(Status::CODE_400, 1001, e.what());
        } catch (...) {
            logFailure("PATCH /api/v1/items/{itemId}/status", "unexpected error");
            return createErrorResponse(Status::CODE_500, 5000, "系统内部错误");
        }
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif
