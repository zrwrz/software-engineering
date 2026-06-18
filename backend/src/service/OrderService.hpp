#ifndef ORDER_SERVICE_HPP
#define ORDER_SERVICE_HPP

#include "dto/request/AuditReservationRequest.hpp"
#include "dto/request/CreateReservationRequest.hpp"
#include "dto/request/RegisterReturnRequest.hpp"
#include "dto/request/ReservationListRequest.hpp"
#include "dto/response/AuditReservationResponse.hpp"
#include "dto/response/CancelReservationResponse.hpp"
#include "dto/response/CreateReservationResponse.hpp"
#include "dto/response/MarkOverdueResponse.hpp"
#include "dto/response/RegisterBorrowResponse.hpp"
#include "dto/response/RegisterReturnResponse.hpp"
#include "dto/response/ReservationDetailedResponse.hpp"
#include "dto/response/ReservationListResponse.hpp"

#include <string>

/**
 * 预约借还业务服务
 */
class OrderService {
  public:
    /**
     * @brief 创建预约订单
     */
    static oatpp::Object<CreateReservationResponse> createReservation(int64_t userId, const oatpp::Object<CreateReservationRequest>& request);

    /**
     * @brief 获取当前用户的预约/订单列表
     */
    static oatpp::Object<ReservationListResponse> getMyOrders(int64_t userId, const oatpp::Object<ReservationListRequest>& request);

    /**
     * @brief 获取当前用户的预约/订单列表
     */
    static oatpp::Object<ReservationListResponse> getMyOrders(int64_t userId, int32_t page, int32_t size, const std::string& status);

    /**
     * @brief 获取订单列表（管理员权限）
     */
    static oatpp::Object<ReservationListResponse> getOrderList(const oatpp::Object<ReservationListRequest>& request);

    /**
     * @brief 获取订单列表（管理员权限）
     */
    static oatpp::Object<ReservationListResponse> getOrderList(int32_t page, int32_t size, const std::string& status);

    /**
     * @brief 获取订单详情
     */
    static oatpp::Object<ReservationDetailedResponse> getOrderDetailed(int64_t orderId);

    /**
     * @brief 取消预约（用户操作）
     */
    static oatpp::Object<CancelReservationResponse> cancelReservation(int64_t currentUserId, int64_t orderId);

    /**
     * @brief 审核预约（管理员操作）
     */
    static oatpp::Object<AuditReservationResponse> auditReservation(int64_t operatorId, int64_t orderId, const oatpp::Object<AuditReservationRequest>& request);

    /**
     * @brief 登记借出
     */
    static oatpp::Object<RegisterBorrowResponse> registerBorrow(int64_t operatorId, int64_t orderId);

    /**
     * @brief 登记归还
     */
    static oatpp::Object<RegisterReturnResponse> registerReturn(int64_t operatorId, int64_t orderId, const oatpp::Object<RegisterReturnRequest>& request);

    /**
     * @brief 标记逾期任务（供定时任务调用）
     */
    static oatpp::Object<MarkOverdueResponse> markOverdue();
};

#endif
