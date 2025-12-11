#ifndef FILEHANDLERS_H
#define FILEHANDLERS_H

#include "../service/fileservice.h"
#include "httplib.h"

namespace handlers {

class FileHandlers {
public:
  explicit FileHandlers(service::FileService& fileService);

  // Регистрация всех обработчиков на сервере
  void registerRoutes(httplib::Server& server);

private:
  // GET /health - проверка здоровья сервиса
  void handleHealth(const httplib::Request& req, httplib::Response& res);

  // POST /files - загрузка файла
  void handleUpload(const httplib::Request& req, httplib::Response& res);

  // GET /files/:id - информация о файле
  void handleGetFile(const httplib::Request& req, httplib::Response& res);

  // GET /files/:id/content - скачивание файла
  void handleDownload(const httplib::Request& req, httplib::Response& res);

  // GET /files/hash/:hash - поиск по хэшу
  void handleFindByHash(const httplib::Request& req, httplib::Response& res);

  // Вспомогательные методы
  void sendError(httplib::Response& res, int status, const std::string& message);
  void sendJson(httplib::Response& res, int status, const std::string& json);

  service::FileService& fileService_;
};

}

#endif //FILEHANDLERS_H
