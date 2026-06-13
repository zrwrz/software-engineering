#include "controller/AdminController.hpp"
#include "controller/AuthController.hpp"
#include "controller/CompensationController.hpp"
#include "controller/CreditController.hpp"
#include "controller/ItemController.hpp"
#include "controller/OrderController.hpp"
#include "controller/StatsController.hpp"
#include "service/UserService.hpp"

#include "oatpp/json/ObjectMapper.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"

#include <iostream>
#include <memory>
#include <cstdlib>

int main() {
    // 1. 创建 ObjectMapper（JSON 序列化）
    auto objectMapper = std::make_shared<oatpp::json::ObjectMapper>();

    // 2. 创建路由器
    auto router = oatpp::web::server::HttpRouter::createShared();
    const std::string secret = "jiewu-secret";

    router->addController(AuthController::createShared(
        objectMapper,
        secret,
        std::make_shared<UserService>()
    ));
    router->addController(ItemController::createShared(objectMapper, secret));
    router->addController(OrderController::createShared(objectMapper, secret));
    router->addController(AdminController::createShared(objectMapper, secret));
    router->addController(CreditController::createShared(objectMapper, secret));
    router->addController(CompensationController::createShared(objectMapper, secret));
    router->addController(StatsController::createShared(objectMapper, secret));

    std::cerr << "控制器创建成功！" << std::endl;

    // 3. 监听端口 8081
    auto connectionProvider = oatpp::network::tcp::server::ConnectionProvider::createShared(
        {"0.0.0.0", 8080, oatpp::network::Address::IP_4}
    );

    std::cerr << "端口监听设置成功！" << std::endl;

    // 4. 创建连接处理器
    auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);

    std::cerr << "连接处理器创建成功！" << std::endl;

    // 5. 创建并启动服务器
    oatpp::network::Server server(connectionProvider, connectionHandler);
    OATPP_LOGi("Server", "Running on http://localhost:8080");
    server.run();
    return 0;
}
