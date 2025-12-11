#include "config.h"
#include <cstdlib>

namespace config {

Config& Config::instance() {
  static Config config;
  return config;
}

const ServerConfig& Config::server() const {
  return server_;
}

Config::Config() {
  server_.port = std::stoi(getEnv("SERVICE_PORT", "8080"));
  server_.fileServiceUrl = getEnv("FILE_SERVICE_URL", "http://file-storing-service:8081");
  server_.analysisServiceUrl = getEnv("ANALYSIS_SERVICE_URL", "http://file-analysis-service:8082");
}

std::string Config::getEnv(const char* name, const char* defaultValue) {
  const char* value = std::getenv(name);
  return value ? value : defaultValue;
}

}