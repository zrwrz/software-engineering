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
#include <cstdint>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include "service/OrderService.hpp"

namespace {
std::string getEnvOrDefault(const char* key, const std::string& defaultValue) {
    const char* value = std::getenv(key);
    if (value == nullptr || value[0] == '\0') {
        return defaultValue;
    }
    return std::string(value);
}

v_uint16 parseServerPort() {
    const std::string rawPort = getEnvOrDefault("JIEWU_SERVER_PORT", "8081");
    try {
        int port = std::stoi(rawPort);
        if (port < 1 || port > 65535) {
            throw std::runtime_error("JIEWU_SERVER_PORT out of range");
        }
        return static_cast<v_uint16>(port);
    } catch (...) {
        throw std::runtime_error("Invalid JIEWU_SERVER_PORT: " + rawPort);
    }
}

std::string loadJwtSecret() {
    std::string secret = getEnvOrDefault("JIEWU_JWT_SECRET", "");
    if (secret.empty()) {
        // Dev fallback to avoid insecure "/api/v1" secret and keep local startup simple.
        secret = "dev-only-jwt-secret-change-me-2026";
        OATPP_LOGW("Config", "JIEWU_JWT_SECRET is empty, using development fallback secret");
    }
    if (secret.size() < 16) {
        throw std::runtime_error("JIEWU_JWT_SECRET must be at least 16 characters");
    }
    return secret;
}
} // namespace

void startOverdueChecker() {
    std::thread([]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::hours(1));
            try {
                OrderService::markOverdue();
                OATPP_LOGI("Scheduler", "markOverdue executed");
            } catch (const std::exception& e) {
                OATPP_LOGE("Scheduler", "markOverdue failed: %s", e.what());
            }
        }
    }).detach();

    std::cout << "[Addition] Overdue checker thread started!" << std::endl;
}

void run() {
    std::cout << "[1] run begin" << std::endl;

    auto router = oatpp::web::server::HttpRouter::createShared();
    std::cout << "[2] router created" << std::endl;

    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    std::cout << "[3] objectMapper created" << std::endl;

    const std::string jwtSecret = loadJwtSecret();

    auto userService = std::make_shared<UserService>();
    std::cout << "[4] userService created" << std::endl;

    router->addController(AuthController::createShared(objectMapper, jwtSecret, userService));
    std::cout << "[5] auth controller added" << std::endl;

    router->addController(ItemController::createShared(objectMapper, jwtSecret));
    std::cout << "[6] item controller added" << std::endl;

    router->addController(OrderController::createShared(objectMapper, jwtSecret));
    std::cout << "[7] order controller added" << std::endl;

    router->addController(AdminController::createShared(objectMapper, jwtSecret));
    router->addController(CreditController::createShared(objectMapper, jwtSecret));
    router->addController(CompensationController::createShared(objectMapper, jwtSecret));
    router->addController(StatsController::createShared(objectMapper, jwtSecret));
    std::cout << "[8] all controllers added" << std::endl;

    auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
    std::cout << "[9] connectionHandler created" << std::endl;

    const v_uint16 port = parseServerPort();
    const std::string bindHost = getEnvOrDefault("JIEWU_BIND_HOST", "127.0.0.1");

    auto connectionProvider = oatpp::network::tcp::server::ConnectionProvider::createShared(
        {bindHost, port, oatpp::network::Address::IP_4}
    );
    std::cout << "[10] connectionProvider created" << std::endl;

    oatpp::network::Server server(connectionProvider, connectionHandler);

    std::cout << "Running on http://localhost:" << port << std::endl;

    server.run();
}

int main() {
    oatpp::base::Environment::init();

    startOverdueChecker();

    run();

    oatpp::base::Environment::destroy();

    return 0;
}