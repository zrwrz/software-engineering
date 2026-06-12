#ifndef REGISTER_RETURN_REQUEST_HPP
#define REGISTER_RETURN_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 登记归还请求体
 */
class RegisterReturnRequest : public oatpp::DTO {
    DTO_INIT(RegisterReturnRequest, DTO)

    DTO_FIELD(String, returnRemark);
    DTO_FIELD(Boolean, needCompensation);
};

#include OATPP_CODEGEN_END(DTO)

#endif