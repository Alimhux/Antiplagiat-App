#include "config/config.h"
#include "db/database.h"
#include "repository/reportrepository.h"
#include "clients/fileserviceclient.h"
#include "service/analysisservice.h"
#include "handlers/analysishandlers.h"
#include "httplib.h"
#include <iostream>

int main() {
  try {
    // 1. Загружаем конфигурацию
    const auto& cfg = config::Config::instance();
    std::cout << "[Main] Configuration loaded" << std::endl;
    std::cout << "[Main] Server port: " << cfg.server().port << std::endl;
    std::cout << "[Main] File service URL: " << cfg.server().fileServiceUrl << std::endl;

    // 2. Подключаемся к БД
    db::Database database(cfg.database().connectionString());

    // 3. Создаём слои приложения
    repository::ReportRepository reportRepo(database);
    clients::FileServiceClient fileClient(cfg.server().fileServiceUrl);
    service::AnalysisService analysisService(reportRepo, fileClient);
    handlers::AnalysisHandlers analysisHandlers(analysisService, fileClient);

    // 4. Настраиваем HTTP сервер
    httplib::Server server;
    analysisHandlers.registerRoutes(server);

    // 5. Запускаем
    std::cout << "[Main] File Analysis Service starting on port " << cfg.server().port << std::endl;
    server.listen("0.0.0.0", cfg.server().port);

  } catch (const std::exception& e) {
    std::cerr << "[Main] Fatal error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}