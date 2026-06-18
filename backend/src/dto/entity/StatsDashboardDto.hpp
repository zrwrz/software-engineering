#ifndef STATS_DASHBOARD_DTO_HPP
#define STATS_DASHBOARD_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "dto/entity/TopBorrowItemDto.hpp"
#include "dto/entity/CreditDistributionDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * 统计看板 DTO
 */
class StatsDashboardDto : public oatpp::DTO {
    DTO_INIT(StatsDashboardDto, DTO)

    DTO_FIELD(Int32, todayReservationCount);
    DTO_FIELD(Int32, borrowingCount);
    DTO_FIELD(Int32, overdueCount);
    DTO_FIELD(Vector<Object<TopBorrowItemDto>>, topBorrowItems);
    DTO_FIELD(Object<CreditDistributionDto>, creditDistribution);
};

#include OATPP_CODEGEN_END(DTO)

#endif
