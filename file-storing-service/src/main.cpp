#include "config/config.h"
#include "db/database.h"
#include "repository/filerepository.h"
#include "service/fileservice.h"
#include "handlers/filehandlers.h"
#include "httplib.h"
#include <iostream>

int main() {
  try {
    // 1. Загружаем конфигурацию
    const auto& cfg = config::Config::instance();
    std::cout << "[Main] Configuration loaded" << std::endl;
    std::cout << "[Main] Server port: " << cfg.server().port << std::endl;
    std::cout << "[Main] Upload path: " << cfg.server().uploadPath << std::endl;

    // 2. Подключаемся к БД
    db::Database database(cfg.database().connectionString());

    // 3. Создаём слои приложения
    repository::FileRepository fileRepo(database);
    service::FileService fileService(fileRepo, cfg.server().uploadPath);
    handlers::FileHandlers fileHandlers(fileService);

    // 4. Настраиваем HTTP сервер
    httplib::Server server;
    fileHandlers.registerRoutes(server);

    // 5. Запускаем
    std::cout << "[Main] File Storing Service starting on port " << cfg.server().port << std::endl;
    server.listen("0.0.0.0", cfg.server().port);
    std::cout << "Server in file-storing-service stopped" << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "[Main] Fatal error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}