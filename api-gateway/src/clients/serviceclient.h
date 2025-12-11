#ifndef SERVICECLIENT_H
#define SERVICECLIENT_H

#include "httplib.h"
#include <string>
#include <optional>
#include <memory>

namespace clients {

// Результат HTTP запроса
struct HttpResponse {
  int status;
  std::string body;
  bool success;
};

class ServiceClient {
public:
  explicit ServiceClient(const std::string& baseUrl);

  // GET запрос
  HttpResponse get(const std::string& path);

  // POST запрос с JSON body
  HttpResponse post(const std::string& path, const std::string& jsonBody);

private:
  std::pair<std::string, int> parseUrl(const std::string& url);
  httplib::Client createClient();

  std::string host_;
  int port_;
};

}


#endif //SERVICECLIENT_H
