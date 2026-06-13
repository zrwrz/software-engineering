#ifndef CREDIT_DISTRIBUTION_DTO_HPP
#define CREDIT_DISTRIBUTION_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class CreditDistributionDto : public oatpp::DTO {
    DTO_INIT(CreditDistributionDto, DTO)

    DTO_FIELD(Int64, excellent);
    DTO_FIELD(Int64, normal);
    DTO_FIELD(Int64, risk);
};

#include OATPP_CODEGEN_END(DTO)

#endif