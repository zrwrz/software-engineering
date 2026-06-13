#ifndef REGISTER_BORROW_RESPONSE_HPP
#define REGISTER_BORROW_RESPONSE_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief 登记借出操作结果对象
 */
class RegisterBorrowResponse : public oatpp::DTO {
    DTO_INIT(RegisterBorrowResponse, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(String, status);
    DTO_FIELD(String, pickupTime);
};

#include OATPP_CODEGEN_END(DTO)

#endif