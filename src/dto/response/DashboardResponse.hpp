#ifndef DASHBOARD_RESPONSE_HPP
#define DASHBOARD_RESPONSE_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"
#include "dto/entity/TopBorrowItemInfoDto.hpp"
#include "dto/entity/CreditDistributionDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class DashboardResponse : public oatpp::DTO {
    DTO_INIT(DashboardResponse, DTO)

    DTO_FIELD(Int64, todayReservationCount);
    DTO_FIELD(Int64, borrowingCount);
    DTO_FIELD(Int64, overdueCount);
    DTO_FIELD(Vector<Object<TopBorrowItemInfoDto>>, topBorrowItems);
    DTO_FIELD(Object<CreditDistributionDto>, creditDistribution);
};

#include OATPP_CODEGEN_END(DTO)

#endif