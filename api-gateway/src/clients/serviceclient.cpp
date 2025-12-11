#include "serviceclient.h"

#include <iostream>

namespace clients {

ServiceClient::ServiceClient(const std::string& baseUrl) {
  auto [h, p] = parseUrl(baseUrl);
  host_ = h;
  port_ = p;
}

HttpResponse ServiceClient::get(const std::string& path) {
  auto client = createClient();
  auto response = client.Get(path.c_str());

  if (!response) {
    return {503, R"({"error":"Service unavailable"})", false};
  }

  return {response->status, response->body, true};
}

HttpResponse ServiceClient::post(const std::string& path, const std::string& jsonBody) {
  auto client = createClient();
  auto response = client.Post(path.c_str(), jsonBody, "application/json");

  if (!response) {
    return {503, R"({"error":"Service unavailable"})", false};
  }

  return {response->status, response->body, true};
}

std::pair<std::string, int> ServiceClient::parseUrl(const std::string& url) {
  std::string host = "localhost";
  int port = 80;

  std::string cleanUrl = url;
  if (cleanUrl.substr(0, 7) == "http://") {
    cleanUrl = cleanUrl.substr(7);
  }

  size_t colonPos = cleanUrl.find(':');
  if (colonPos != std::string::npos) {
    host = cleanUrl.substr(0, colonPos);
    port = std::stoi(cleanUrl.substr(colonPos + 1));
  } else {
    host = cleanUrl;
  }

  return {host, port};
}

httplib::Client ServiceClient::createClient() {
  httplib::Client client(host_, port_);
  client.set_connection_timeout(5);
  client.set_read_timeout(30);
  return client;
}

}