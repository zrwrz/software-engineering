#ifndef CHANGE_PASSWORD_SUCCESS_DTO_HPP
#define CHANGE_PASSWORD_SUCCESS_DTO_HPP

#include "oatpp/macro/codegen.hpp"
#include "oatpp/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class SuccessDto : public oatpp::DTO {
    DTO_INIT(SuccessDto, DTO)

    /**
     * @brief 操作是否成功
     */
    DTO_FIELD(Boolean, success);
};

#include OATPP_CODEGEN_END(DTO)

#endif // SUCCESS_DTO_HPP