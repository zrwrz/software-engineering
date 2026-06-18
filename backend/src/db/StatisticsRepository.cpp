#include "db/StatisticsRepository.hpp"
#include "db/Database.hpp"

#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

namespace {
int queryInt(sql::Connection* conn, const std::string& sqlText) {
    std::unique_ptr<sql::Statement> stmt(conn->createStatement());
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery(sqlText));
    if (rs->next()) return rs->getInt(1);
    return 0;
}
}

oatpp::Object<StatsDashboardDto> StatisticsRepository::getDashboard() {
    auto conn = Database::getConnection();
    auto dashboard = StatsDashboardDto::createShared();
    dashboard->todayReservationCount = queryInt(conn.get(), "SELECT COUNT(*) FROM orders WHERE status = 'CREATED'");
    dashboard->borrowingCount = queryInt(conn.get(), "SELECT COUNT(*) FROM orders WHERE status = 'BORROWED'");
    dashboard->overdueCount = queryInt(conn.get(), "SELECT COUNT(*) FROM orders WHERE status IN ('APPROVED', 'BORROWED', 'OVERDUE')");

    dashboard->topBorrowItems = oatpp::Vector<oatpp::Object<TopBorrowItemDto>>::createShared();
    std::unique_ptr<sql::PreparedStatement> topStmt(conn->prepareStatement(
        "SELECT i.id item_id, i.name, COUNT(o.id) borrow_count "
        "FROM orders o JOIN items i ON o.item_id = i.id "
        "WHERE o.status IN ('BORROWED', 'RETURNED', 'OVERDUE') "
        "GROUP BY i.id, i.name ORDER BY borrow_count DESC LIMIT 10"
    ));
    std::unique_ptr<sql::ResultSet> topRs(topStmt->executeQuery());
    while (topRs->next()) {
        auto item = TopBorrowItemDto::createShared();
        item->itemId = topRs->getInt64("item_id");
        item->name = topRs->getString("name").c_str();
        item->count = topRs->getInt("borrow_count");
        dashboard->topBorrowItems->push_back(item);
    }

    auto distribution = CreditDistributionDto::createShared();
    distribution->excellent = queryInt(conn.get(), "SELECT COUNT(*) FROM users WHERE credit_score >= 90");
    distribution->normal = queryInt(conn.get(), "SELECT COUNT(*) FROM users WHERE credit_score >= 60 AND credit_score < 90");
    distribution->risk = queryInt(conn.get(), "SELECT COUNT(*) FROM users WHERE credit_score < 60");
    dashboard->creditDistribution = distribution;

    return dashboard;
}
