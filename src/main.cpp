#include "controller/AuthController.hpp"
#include "service/UserService.hpp"
#include <memory>
#include <memory>
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/json/ObjectMapper.hpp"

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
    
    // 3. 监听端口 8080
    auto connectionProvider = oatpp::network::tcp::server::ConnectionProvider::createShared(
        {"0.0.0.0", 8080, oatpp::network::Address::IP_4}
    );
    
    // 4. 创建连接处理器
    auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
    
    // 5. 创建并启动服务器
    oatpp::network::Server server(connectionProvider, connectionHandler);
    OATPP_LOGi("Server", "Running on http://localhost:8080");
    server.run();
    
    return 0;
}
