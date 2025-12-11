#include "config.h"
#include <cstdlib>

namespace config {

std::string DatabaseConfig::connectionString() const {
  return "host=" + host + " port=" + port + " dbname=" + name +
         " user=" + user + " password=" + password;
}

Config& Config::instance() {
  static Config config;
  return config;
}

const DatabaseConfig& Config::database() const {
  return db_;
}

const ServerConfig& Config::server() const {
  return server_;
}

Config::Config() {
  // Database config
  db_.host = getEnv("DB_HOST", "localhost");
  db_.port = getEnv("DB_PORT", "5432");
  db_.name = getEnv("DB_NAME", "files_db");
  db_.user = getEnv("DB_USER", "postgres");
  db_.password = getEnv("DB_PASSWORD", "postgres");

  // Server config
  server_.port = std::stoi(getEnv("SERVICE_PORT", "8081"));
  server_.uploadPath = getEnv("UPLOAD_PATH", "/app/uploads/");
}

std::string Config::getEnv(const char* name, const char* defaultValue) {
  const char* value = std::getenv(name);
  return value ? value : defaultValue;
}

}
