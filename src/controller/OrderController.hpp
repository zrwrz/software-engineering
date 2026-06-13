#ifndef ORDER_CONTROLLER_HPP
#define ORDER_CONTROLLER_HPP

#include "dto/request/CreateReservationRequest.hpp"
#include "dto/response/CreateReservationResponse.hpp"

#include "dto/request/ReservationListRequest.hpp"
#include "dto/response/ReservationListResponse.hpp"

#include "dto/response/ReservationDetailedResponse.hpp"

#include "dto/response/CancelReservationResponse.hpp"

#include "dto/request/AuditReservationRequest.hpp"
#include "dto/response/AuditReservationResponse.hpp"

#include "dto/response/RegisterBorrowResponse.hpp"

#include "dto/request/RegisterReturnRequest.hpp"
#include "dto/response/RegisterReturnResponse.hpp"

#include "dto/response/MarkOverdueResponse.hpp"

#include "dto/response/Response.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/macro/codegen.hpp"
#include "service/OrderService.hpp"
#include "service/UserService.hpp"
#include "utils/jwt.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class OrderController : public oatpp::web::server::api::ApiController {
  private:
	const std::string secret;
	std::shared_ptr<OrderService> order_service;
	std::shared_ptr<UserService> user_service;

	/**
	 * @brief 构建统一格式的错误响应（私有辅助方法）
	 * @param status HTTP 状态码
	 * @param code 业务错误码
	 * @param message 错误提示信息
	 * @return 统一格式的 HTTP 响应
	 */
	std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> createErrorResponse(
		Status status, Int32 code, const oatpp::String& message) {
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
		Status status, const oatpp::Any& data) {
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
	OrderController(
		const std::shared_ptr<ObjectMapper>& objectMapper,
		const std::string &secret,
		const std::shared_ptr<OrderService>& order_service = nullptr,
		const std::shared_ptr<UserService>& user_service = nullptr
	) : oatpp::web::server::api::ApiController(objectMapper),
		secret(secret),
		order_service(order_service),
		user_service(user_service) {}

	static std::shared_ptr<OrderController> createShared(
		const std::shared_ptr<ObjectMapper>& objectMapper,
		const std::string &secret,
		const std::shared_ptr<OrderService>& order_service = nullptr,
		const std::shared_ptr<UserService>& user_service = nullptr) {
		return std::make_shared<OrderController>(objectMapper, secret, order_service, user_service);
	}

	/**
	 * @brief 创建预约订单
	 */
	ENDPOINT("POST", "api/v1/orders/reservations", createReservation,
		HEADER(String, authorization, "Authorization"),
		BODY_DTO(Object<CreateReservationRequest>, request)) {
		
		auto userId = getUserIdFromAuth(authorization);
		if (userId == 0) {
			return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
		}

		if (!request) {
			return createErrorResponse(Status::CODE_400, 1001, "请求体不能为空");
		}

		if (request->quantity <= 0) {
			return createErrorResponse(Status::CODE_400, 1001, "预约数量必须大于 0");
		}

		try {
			auto result = order_service->createReservation(userId, request);
			return createOKResponse(Status::CODE_200, result);
		} catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
			return createErrorResponse(Status::CODE_500, 5000, std::string("创建预约订单失败: ") + e.what());
		}
	}

	/**
	 * @brief 获取当前用户的预约/订单列表
	 */
	ENDPOINT("GET", "api/v1/orders/my", getMyOrders,
		HEADER(String, authorization, "Authorization"),
		QUERIES(Object<ReservationListRequest>, queryParams)) {

		// 1. 解析 JWT Token 并校验用户身份
		auto userId = getUserIdFromAuth(authorization);
		if (userId == 0) {
			return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
		}

		// 2. 校验查询参数是否为空
		if (!queryParams) {
			return createErrorResponse(Status::CODE_400, 1001, "查询参数不能为空");
		}

		// 3. 调用 Service 层获取当前用户的订单列表
		try {
			auto result = order_service->getMyOrders(userId, queryParams);
			return createOKResponse(Status::CODE_200, result);
		} catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
			// 4. 捕获异常并返回统一格式的错误响应
			return createErrorResponse(Status::CODE_500, 5000, std::string("获取我的订单列表失败: ") + e.what());
		}
	}

	/**
	 * @brief 获取订单列表（管理员）
	 */
	ENDPOINT("GET", "api/v1/orders", getOrderList,
		HEADER(String, authorization, "Authorization"),
		QUERIES(Object<ReservationListRequest>, queryParams)) {

		// 1. 解析 JWT Token 并校验用户身份
		auto userId = getUserIdFromAuth(authorization);
		if (userId == 0) {
			return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
		}

		// 2. 校验当前用户是否为管理员
		if (!user_service->isAdmin(userId)) {
			return createErrorResponse(Status::CODE_403, 1003, "无权限访问，仅管理员可操作");
		}

		// 3. 校验查询参数是否为空
		if (!queryParams) {
			return createErrorResponse(Status::CODE_400, 1001, "查询参数不能为空");
		}

		// 4. 调用 Service 层获取全量订单列表（供管理员使用）
		try {
			auto result = order_service->getOrderList(queryParams);
			return createOKResponse(Status::CODE_200, result);
		} catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
			// 5. 捕获异常并返回统一格式的错误响应
			return createErrorResponse(Status::CODE_500, 5000, std::string("获取订单列表失败: ") + e.what());
		}
	}

	/**
	 * @brief 获取订单详情
	 */
	ENDPOINT("GET", "api/v1/orders/{orderId}", getOrderDetailed,
		HEADER(String, authorization, "Authorization"),
		PATH(Int64, orderId)) {

		// 1. 解析 JWT Token 并校验用户身份
		auto userId = getUserIdFromAuth(authorization);
		if (userId == 0) {
			return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
		}

		// 2. 调用 Service 层获取订单详情
		try {
			auto result = order_service->getOrderDetailed(orderId);
			
			// 3. 校验订单是否存在
			if (!result) {
				return createErrorResponse(Status::CODE_404, 2001, "订单不存在");
			}

			return createOKResponse(Status::CODE_200, result);
		} catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
			// 5. 捕获异常并返回统一格式的错误响应
			return createErrorResponse(Status::CODE_500, 5000, std::string("获取订单详情失败: ") + e.what());
		}
	}

	/**
	 * @brief 取消预约（用户操作）
	 */
	ENDPOINT("PATCH", "api/v1/orders/{orderId}/cancel", cancelReservation,
		HEADER(String, authorization, "Authorization"),
		PATH(Int64, reservationId)) {

		auto userId = getUserIdFromAuth(authorization);
		if (userId == 0) {
			return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
		}

		try {
			auto order = order_service->getOrderDetailed(reservationId);
			if (!order) {
				return createErrorResponse(Status::CODE_404, 2001, "订单不存在");
			}
			if (order->userId != userId) {
				return createErrorResponse(Status::CODE_403, 1003, "无权限操作该订单");
			}
			if (order->status != "created") {
				return createErrorResponse(Status::CODE_400, 2002, "当前订单状态不允许取消");
			}

			bool isSuccess = order_service->cancelReservation(reservationId);
			if (isSuccess) {
				// 修正：按照文档要求，返回包含订单ID及更新后状态(CANCELLED)的响应对象
				auto response = CancelReservationResponse::createShared();
				response->id = reservationId;
				response->status = "CANCELLED";
				return createOKResponse(Status::CODE_200, response);
			} else {
				return createErrorResponse(Status::CODE_500, 5000, "取消预约失败，请稍后重试");
			}

		} catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
			return createErrorResponse(Status::CODE_500, 5000, std::string("取消预约失败: ") + e.what());
		}
	}

	/**
	 * @brief 审核预约（管理员操作）
	 */
	ENDPOINT("PATCH", "api/v1/orders/{orderId}/audit", auditReservation,
		HEADER(String, authorization, "Authorization"),
		PATH(Int64, orderId),
		BODY_DTO(Object<AuditReservationRequest>, request)) {

		// 1. 解析 JWT Token 并校验用户身份
		auto userId = getUserIdFromAuth(authorization);
		if (userId == 0) {
			return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
		}

		// 2. 校验当前用户是否为管理员
		if (!user_service->isAdmin(userId)) {
			return createErrorResponse(Status::CODE_403, 1003, "无权限访问，仅管理员可操作");
		}

		// 3. 校验请求体及 action 字段合法性
		if (!request || !request->action) {
			return createErrorResponse(Status::CODE_400, 1001, "请求体或 action 不能为空");
		}
		if (request->action != "APPROVE" && request->action != "REJECT") {
			return createErrorResponse(Status::CODE_400, 1001, "无效的审核动作，仅允许 APPROVE 或 REJECT");
		}

		try {
			// 4. 获取订单详情，校验状态是否允许审核（仅 CREATED 状态可审核）
			auto order = order_service->getOrderDetailed(orderId);
			if (!order) {
				return createErrorResponse(Status::CODE_404, 2001, "订单不存在");
			}
			if (order->status != "CREATED") {
				return createErrorResponse(Status::CODE_400, 2002, "当前订单状态不允许审核");
			}

			// 5. 调用 Service 层执行审核逻辑
			bool isSuccess = order_service->auditReservation(orderId, request);
			
			// 6. 根据执行结果构建响应
			if (isSuccess) {
				auto response = AuditReservationResponse::createShared();
				
				// 构建返回 id, status, remark
				response->id = orderId;
				response->status = (request->action == "APPROVE") ? "APPROVED" : "REJECTED";
				response->remark = request->reason;
				
				return createOKResponse(Status::CODE_200, response);
			} else {
				return createErrorResponse(Status::CODE_500, 5000, "审核操作失败，请稍后重试");
			}

		} catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
			return createErrorResponse(Status::CODE_500, 5000, std::string("审核预约失败: ") + e.what());
		}
	}

	/**
	 * @brief 登记借出（管理员操作）
	 */
	ENDPOINT("PATCH", "api/v1/orders/{orderId}/borrow", registerBorrow,
		HEADER(String, authorization, "Authorization"),
		PATH(Int64, orderId)) {

		// 1. 身份与权限校验
		auto userId = getUserIdFromAuth(authorization);
		if (userId == 0) {
			return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
		}
		if (!user_service->isAdmin(userId)) {
			return createErrorResponse(Status::CODE_403, 1003, "无权限访问，仅管理员可操作");
		}

		try {
			// 2. 获取订单详情并校验状态流转（仅 APPROVED 状态可办理借出）
			auto order = order_service->getOrderDetailed(orderId);
			if (!order) {
				return createErrorResponse(Status::CODE_404, 2001, "订单不存在");
			}
			if (order->status != "APPROVED") {
				return createErrorResponse(Status::CODE_400, 2002, "当前订单状态不允许办理借出");
			}

			// 3. 调用 Service 层执行借出逻辑（Service 内部更新数据库并获取数据库时间）
			auto response = order_service->registerBorrow(orderId);
			
			// 4. 判断 Service 是否成功执行（防止数据库异常导致空指针）
			if (response) {
				return createOKResponse(Status::CODE_200, response);
			} else {
				return createErrorResponse(Status::CODE_500, 5000, "登记借出失败，请稍后重试");
			}

		} catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
			return createErrorResponse(Status::CODE_500, 5000, std::string("登记借出失败: ") + e.what());
		}
	}

	/**
	 * @brief 登记归还（管理员操作）
	 */
	ENDPOINT("PATCH", "api/v1/orders/{orderId}/return", registerReturn,
		HEADER(String, authorization, "Authorization"),
		PATH(Int64, orderId),
		BODY_DTO(Object<RegisterReturnRequest>, request)) {

		// 1. 身份与权限校验
		auto userId = getUserIdFromAuth(authorization);
		if (userId == 0) {
			return createErrorResponse(Status::CODE_401, 1002, "未授权或 Token 无效");
		}
		if (!user_service->isAdmin(userId)) {
			return createErrorResponse(Status::CODE_403, 1003, "无权限访问，仅管理员可操作");
		}

		try {
			// 2. 获取订单详情并校验状态流转（仅 BORROWED 状态可办理归还）
			auto order = order_service->getOrderDetailed(orderId);
			if (!order) {
				return createErrorResponse(Status::CODE_404, 2001, "订单不存在");
			}
			if (order->status != "BORROWED") {
				return createErrorResponse(Status::CODE_400, 2002, "当前订单状态不允许办理归还");
			}

			// 3. 调用 Service 层执行归还逻辑（Service 内部落库、获取时间并组装 DTO）
			auto response = order_service->registerReturn(orderId, request);
			
			// 4. 判断 Service 是否成功执行
			if (response) {
				return createOKResponse(Status::CODE_200, response);
			} else {
				return createErrorResponse(Status::CODE_500, 5000, "登记归还失败，请稍后重试");
			}

		} catch (const std::runtime_error& e) {
            return createErrorResponse(Status::CODE_400, 4001, e.what());
        } catch (const std::exception& e) {
			return createErrorResponse(Status::CODE_500, 5000, std::string("登记归还失败: ") + e.what());
		}
	}

	/**
	 * @brief 标记逾期任务（供定时任务调用）
	 */
	ENDPOINT("POST", "api/v1/internal/orders/mark-overdue", markOverdue,
		HEADER(String, authorization, "Authorization")) {
		// TODO: 实现标记逾期逻辑
		return createErrorResponse(Status::CODE_501, 5001, "Not Implemented");
	}
};

#endif