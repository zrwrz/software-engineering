#ifndef RESERVATION_LIST_REQUEST_HPP
#define RESERVATION_LIST_REQUEST_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 预约列表查询参数
 * 可能同时用于自己创建的查询，或者管理员查询所有正在检查的
 */
class ReservationListRequest : public oatpp::DTO {
    DTO_INIT(ReservationListRequest, DTO)

    DTO_FIELD(Int32, page) = 1;
    DTO_FIELD(Int32, size) = 10;
    DTO_FIELD(String, status) = nullptr;
};

#include OATPP_CODEGEN_END(DTO)

#endif