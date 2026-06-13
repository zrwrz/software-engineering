#ifndef UPDATE_COMPENSATE_STATUS_REQUEST_HPP
#define UPDATE_COMPENSATE_STATUS_REQUEST_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class UpdateCompensateStatusRequest : public oatpp::DTO {
    DTO_INIT(UpdateCompensateStatusRequest, DTO)

    DTO_FIELD(String, status);
};

#include OATPP_CODEGEN_END(DTO)

#endif