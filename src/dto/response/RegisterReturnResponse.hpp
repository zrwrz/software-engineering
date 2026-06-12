#ifndef REGISTER_RETURN_RESPONSE_HPP
#define REGISTER_RETURN_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 登记归还响应体
 */
class RegisterReturnResponse : public oatpp::DTO {
    DTO_INIT(RegisterReturnResponse, DTO)

    DTO_FIELD(Int64, id);         // 订单ID
    DTO_FIELD(String, status);    // 订单状态（如 RETURNED）
    DTO_FIELD(String, returnTime); // 实际归还时间
    DTO_FIELD(Boolean, needCompensation);   
};

#include OATPP_CODEGEN_END(DTO)

#endif