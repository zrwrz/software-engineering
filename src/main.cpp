#include "controller/AuthController.hpp"
#include "controller/ItemController.hpp"
#include "controller/OrderController.hpp"
#include "controller/AdminController.hpp"
#include "controller/CreditController.hpp"
#include "controller/CompensationController.hpp"
#include "controller/StatsController.hpp"

#include "service/UserService.hpp"

#include "oatpp/core/base/Environment.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

void run() {
    std::cout << "[1] run begin" << std::endl;

    auto router = oatpp::web::server::HttpRouter::createShared();
    std::cout << "[2] router created" << std::endl;

    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    std::cout << "[3] objectMapper created" << std::endl;

    const std::string basePath = "/api/v1";

    auto userService = std::make_shared<UserService>();
    std::cout << "[4] userService created" << std::endl;

    router->addController(AuthController::createShared(objectMapper, basePath, userService));
    std::cout << "[5] auth controller added" << std::endl;

    router->addController(ItemController::createShared(objectMapper, basePath));
    std::cout << "[6] item controller added" << std::endl;

    router->addController(OrderController::createShared(objectMapper, basePath));
    std::cout << "[7] order controller added" << std::endl;

    router->addController(AdminController::createShared(objectMapper, basePath));
    router->addController(CreditController::createShared(objectMapper, basePath));
    router->addController(CompensationController::createShared(objectMapper, basePath));
    router->addController(StatsController::createShared(objectMapper, basePath));
    std::cout << "[8] all controllers added" << std::endl;

    auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
    std::cout << "[9] connectionHandler created" << std::endl;

    const char* portEnv = std::getenv("JIEWU_SERVER_PORT");
    const v_uint16 port = (portEnv != nullptr && portEnv[0] != '\0')
        ? static_cast<v_uint16>(std::atoi(portEnv))
        : 8081;

    auto connectionProvider = oatpp::network::tcp::server::ConnectionProvider::createShared(
        {"0.0.0.0", port, oatpp::network::Address::IP_4}
    );
    std::cout << "[10] connectionProvider created" << std::endl;

    oatpp::network::Server server(connectionProvider, connectionHandler);

    std::cout << "Running on http://localhost:" << port << std::endl;

    server.run();
}

int main() {
    oatpp::base::Environment::init();

    run();

    oatpp::base::Environment::destroy();

    return 0;
}