#ifndef RETURN_ORDER_REQUEST_HPP
#define RETURN_ORDER_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 办理归还请求 DTO
 */
class ReturnOrderRequest : public oatpp::DTO {
    DTO_INIT(ReturnOrderRequest, DTO)

    DTO_FIELD(String, returnRemark);
    DTO_FIELD(Boolean, needCompensation) = false;
};

#include OATPP_CODEGEN_END(DTO)

#endif
