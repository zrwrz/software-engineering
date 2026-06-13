#ifndef CREDIT_SERVICE_HPP
#define CREDIT_SERVICE_HPP

#include "dto/request/CreditRecordRequest.hpp"
#include "dto/response/CreditRecordResponse.hpp"

#include "dto/request/UpdateCreditRequest.hpp"
#include "dto/response/UpdateCreditResponse.hpp"

#include "dto/request/CompensateRecordRequest.hpp"
#include "dto/response/CompensateRecordResponse.hpp"

#include "dto/request/CreateCompensateRecordRequest.hpp"
#include "dto/response/CreateCompensateRecordResponse.hpp"

#include "dto/request/UpdateCompensateStatusRequest.hpp"
#include "dto/response/UpdateCompensateStatusResponse.hpp"

#include "dto/response/DashboardResponse.hpp"

class CreditService {
  public:
    /**
     * @brief 查询用户信用记录
     * 
     * @param request 查询请求对象，包含 userId、page、size
     * @return oatpp::Object<CreditRecordResponse> 分页信用记录列表响应对象
     */
    static oatpp::Object<CreditRecordResponse> selectCreditRecords(
        const oatpp::Object<CreditRecordRequest>& request
    );

    /**
     * @brief 手工调整信用分（管理员）
     * 
     * @param request 调整请求对象，包含 userId、changeValue、reason
     * @return oatpp::Object<UpdateCreditResponse> 调整后的信用记录响应
     */
    static oatpp::Object<UpdateCreditResponse> adjustCredit(
        const oatpp::Object<UpdateCreditRequest>& request
    );

    /**
     * @brief 查询赔偿记录（管理员）
     * 
     * @param request 查询请求对象，包含 page、size、status、userId、orderId
     * @return oatpp::Object<CompensateRecordResponse> 分页赔偿记录列表响应对象
     */
    static oatpp::Object<CompensateRecordResponse> selectCompensateRecords(
        const oatpp::Object<CompensateRecordRequest>& request
    );

    /**
     * @brief 新增赔偿记录（管理员）
     * 
     * @param request 创建请求对象，包含 orderId、userId、amount、reason
     * @return oatpp::Object<CreateCompensateRecordResponse> 创建的赔偿记录响应
     */
    static oatpp::Object<CreateCompensateRecordResponse> createCompensateRecord(
        const oatpp::Object<CreateCompensateRecordRequest>& request
    );

    /**
     * @brief 更新赔偿状态（管理员）
     * 
     * @param id 赔偿记录ID
     * @param request 状态更新请求对象，包含 status
     * @return oatpp::Object<UpdateCompensateStatusResponse> 更新后的状态响应
     */
    static oatpp::Object<UpdateCompensateStatusResponse> updateCompensateStatus(
        int64_t id,
        const oatpp::Object<UpdateCompensateStatusRequest>& request
    );

    /**
     * @brief 获取统计看板数据
     * 
     * @return oatpp::Object<DashboardResponse> 统计看板响应对象
     */
    static oatpp::Object<DashboardResponse> getDashboard();
};

#endif