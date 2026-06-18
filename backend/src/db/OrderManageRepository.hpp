#ifndef ORDER_MANAGE_REPOSITORY_HPP
#define ORDER_MANAGE_REPOSITORY_HPP

#include "dto/entity/MarkOverdueResultDto.hpp"
#include "dto/entity/OrderDto.hpp"
#include "dto/entity/StatusResultDto.hpp"
#include "dto/request/ReturnOrderRequest.hpp"
#include "dto/response/OrderListResponse.hpp"
#include <string>

/**
 * 预约借还管理数据访问层
 */
class OrderManageRepository {
  public:
    static oatpp::Object<OrderListResponse> listOrders(int32_t page, int32_t size, const std::string& status);
    static oatpp::Object<OrderDto> getOrderDetail(int64_t orderId);
    static oatpp::Object<StatusResultDto> auditOrder(int64_t operatorId, int64_t orderId, const std::string& action, const std::string& remark);
    static oatpp::Object<StatusResultDto> borrowOrder(int64_t operatorId, int64_t orderId);
    static oatpp::Object<StatusResultDto> returnOrder(int64_t operatorId, int64_t orderId, const oatpp::Object<ReturnOrderRequest>& request);
    static oatpp::Object<MarkOverdueResultDto> markOverdue();
};

#endif
