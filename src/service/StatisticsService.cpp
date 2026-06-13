#include "service/StatisticsService.hpp"
#include "db/StatisticsRepository.hpp"

oatpp::Object<StatsDashboardDto> StatisticsService::getDashboard() {
    return StatisticsRepository::getDashboard();
}
