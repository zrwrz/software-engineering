#ifndef STATISTICS_REPOSITORY_HPP
#define STATISTICS_REPOSITORY_HPP

#include "dto/entity/StatsDashboardDto.hpp"

/**
 * 统计展示数据访问层
 */
class StatisticsRepository {
  public:
    static oatpp::Object<StatsDashboardDto> getDashboard();
};

#endif
