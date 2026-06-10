#ifndef ITEM_CONTROLLER_HPP
#define ITEM_CONTROLLER_HPP

#include "dto/request/ItemListRequest.hpp"
#include "dto/response/ItemListResponse.hpp"

#include "dto/response/ItemDetailedResponse.hpp"

#include "dto/request/CreateItemRequest.hpp"
#include "dto/response/CreateItemResponse.hpp"

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
#include "utils/jwt.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class ItemController : public oatpp::web::server::api::ApiController {
  private:
    const std::string secret;
    std::shared_ptr<ItemService> item_service;

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

  public:
    
    ItemController(
        const std::shared_ptr<ObjectMapper>& objectMapper,
        const std::string &secret,
        const std::shared_ptr<ItemService>& item_service = nullptr
        )
        :
        oatpp::web::server::api::ApiController(objectMapper),
        secret(secret),
        item_service(item_service)
    {}

    static std::shared_ptr<ItemController> createShared(
        const std::shared_ptr<ObjectMapper>& objectMapper,
        const std::string &secret,
        const std::shared_ptr<ItemService>& ItemService = nullptr) {
        return std::make_shared<ItemController>(objectMapper, secret, ItemService);
    }

    ENDPOINT()
};

#endif