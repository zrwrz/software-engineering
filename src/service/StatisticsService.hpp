#ifndef STATISTICS_SERVICE_HPP
#define STATISTICS_SERVICE_HPP

#include "dto/entity/StatsDashboardDto.hpp"

/**
 * 统计展示业务服务
 */
class StatisticsService {
  public:
    static oatpp::Object<StatsDashboardDto> getDashboard();
};

#endif
