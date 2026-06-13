#ifndef UPDATE_COMPENSATE_STATUS_RESPONSE_HPP
#define UPDATE_COMPENSATE_STATUS_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class UpdateCompensateStatusResponse : public oatpp::DTO {
    DTO_INIT(UpdateCompensateStatusResponse, DTO)

    DTO_FIELD(Int64, id);
    DTO_FIELD(String, status);
};

#include OATPP_CODEGEN_END(DTO)

#endif