#ifndef ORDER_SERVICE_HPP
#define ORDER_SERVICE_HPP

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

class OrderService {
  public:
    /**
     * @brief 创建预约订单
     * 
     * @param userId 当前操作用户的ID（从JWT中解析）
     * @param request 包含物品ID、数量、预约起止时间及备注的请求对象
     * @return oatpp::Object<CreateReservationResponse> 包含生成的订单号、初始状态(CREATED)及时间信息的响应对象
     */
    static oatpp::Object<CreateReservationResponse> createReservation(
        int64_t userId, 
        const oatpp::Object<CreateReservationRequest>& request
    );

    /**
     * @brief 获取当前用户的预约/订单列表
     * 
     * @param userId 当前操作用户的ID（从JWT中解析）
     * @param request 包含分页参数(page, size)及可选状态(status)筛选的查询对象
     * @return oatpp::Object<ReservationListResponse> 分页订单列表响应对象，包含list、page、size和total字段
     */
    static oatpp::Object<ReservationListResponse> getMyOrders(
        int64_t userId, 
        const oatpp::Object<ReservationListRequest>& request
    );

    /**
     * @brief 获取订单列表（管理员权限）
     * 
     * @param request 包含分页参数(page, size)及可选状态(status)筛选的查询对象
     * @return oatpp::Object<ReservationListResponse> 分页订单列表响应对象，包含所有用户的订单数据
     */
    static oatpp::Object<ReservationListResponse> getOrderList(
        const oatpp::Object<ReservationListRequest>& request
    );

    /**
     * @brief 获取订单详情
     * 
     * @param orderId 指定的订单ID
     * @return oatpp::Object<ReservationDetailedResponse> 订单详细信息返回体，包含借出/归还时间及备注等完整字段
     */
    static oatpp::Object<ReservationDetailedResponse> getOrderDetailed(
        int64_t orderId
    );

	/**
	 * @brief 取消预约（用户操作）
	 * @param orderId 指定的订单ID，仅CREATED状态可取消
	 * @return bool 返回 true 表示取消成功，false 表示失败（如订单不存在或状态不允许）
	 */
	bool cancelReservation(int64_t orderId);

	/**
	 * @brief 审核预约（管理员操作）
	 * @param orderId 指定的订单ID
	 * @param request 包含审核动作及驳回原因(可选)的请求对象
	 * @return bool 返回 true 表示修改成功。注意表示的是修改成没成功而不是审核通过没通过！
	 */
	bool auditReservation(int64_t orderId, const oatpp::Object<AuditReservationRequest>& request);

    /**
     * @brief 登记借出
     * 
     * @param orderId 指定的订单ID
     * @return oatpp::Object<RegisterBorrowResponse> 包含订单ID、状态(BORROWED)及实际借出时间的响应对象
     */
    static oatpp::Object<RegisterBorrowResponse> registerBorrow(
        int64_t orderId
    );

    /**
     * @brief 登记归还
     * 
     * @param orderId 指定的订单ID
     * @param request 包含物品状态(condition)及归还备注(remark)的请求对象
     * @return oatpp::Object<RegisterReturnResponse> 包含订单ID、状态(RETURNED)及实际归还时间的响应对象
     */
    static oatpp::Object<RegisterReturnResponse> registerReturn(
        int64_t orderId, 
        const oatpp::Object<RegisterReturnRequest>& request
    );

    /**
     * @brief 标记逾期任务（供定时任务调用）
     * 
     * @return oatpp::Object<MarkOverdueResponse> 包含更新的订单数量(updatedCount)及对应订单ID列表(orderIds)的响应对象
     */
    static oatpp::Object<MarkOverdueResponse> markOverdue();
};

#endif