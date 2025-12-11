#include "config/config.h"
#include "clients/serviceclient.h"
#include "handlers/gatewayhandlers.h"
#include "httplib.h"
#include <iostream>

int main() {
  try {
    // 1. Загружаем конфигурацию
    const auto& cfg = config::Config::instance();
    std::cout << "[Main] Configuration loaded" << std::endl;
    std::cout << "[Main] Server port: " << cfg.server().port << std::endl;
    std::cout << "[Main] File service URL: " << cfg.server().fileServiceUrl << std::endl;
    std::cout << "[Main] Analysis service URL: " << cfg.server().analysisServiceUrl << std::endl;

    // 2. Создаём клиенты для сервисов
    clients::ServiceClient fileService(cfg.server().fileServiceUrl);
    clients::ServiceClient analysisService(cfg.server().analysisServiceUrl);

    // 3. Создаём обработчики
    handlers::GatewayHandlers gatewayHandlers(fileService, analysisService);

    // 4. Настраиваем HTTP сервер
    httplib::Server server;
    gatewayHandlers.registerRoutes(server);

    // 5. Запускаем
    std::cout << "[Main] API Gateway starting on port " << cfg.server().port << std::endl;
    server.listen("0.0.0.0", cfg.server().port);

  } catch (const std::exception& e) {
    std::cerr << "[Main] Fatal error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}