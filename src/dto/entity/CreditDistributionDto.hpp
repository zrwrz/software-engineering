#ifndef CREDIT_DISTRIBUTION_DTO_HPP
#define CREDIT_DISTRIBUTION_DTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 信用分布 DTO
 */
class CreditDistributionDto : public oatpp::DTO {
    DTO_INIT(CreditDistributionDto, DTO)

    DTO_FIELD(Int32, excellent);
    DTO_FIELD(Int32, normal);
    DTO_FIELD(Int32, risk);
};

#include OATPP_CODEGEN_END(DTO)

#endif
