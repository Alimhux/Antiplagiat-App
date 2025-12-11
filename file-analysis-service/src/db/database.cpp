#include "database.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <pqxx/pqxx>

namespace db {

Database::Database(const std::string& connectionString, int maxRetries, int retryDelaySeconds)
    : connectionString_(connectionString)
    , maxRetries_(maxRetries)
    , retryDelaySeconds_(retryDelaySeconds)
{
  connect();
}

Database::~Database() = default;

Database::Database(Database&&) noexcept = default;
Database& Database::operator=(Database&&) noexcept = default;

pqxx::connection& Database::connection() {
  if (!conn_ || !conn_->is_open()) {
    reconnect();
  }
  return *conn_;
}

bool Database::isConnected() const {
  return conn_ && conn_->is_open();
}

void Database::connect() {
  std::cout << "[Database] Connecting to database..." << std::endl;

  for (int attempt = 1; attempt <= maxRetries_; ++attempt) {
    try {
      conn_ = std::make_unique<pqxx::connection>(connectionString_);
      std::cout << "[Database] Connected successfully!" << std::endl;
      return;
    } catch (const std::exception& e) {
      std::cerr << "[Database] Connection attempt " << attempt << "/" << maxRetries_
                << " failed: " << e.what() << std::endl;

      if (attempt < maxRetries_) {
        std::this_thread::sleep_for(std::chrono::seconds(retryDelaySeconds_));
      }
    }
  }

  throw std::runtime_error("Failed to connect to database after " +
                            std::to_string(maxRetries_) + " attempts");
}

void Database::reconnect() {
  std::cout << "[Database] Reconnecting..." << std::endl;
  connect();
}

}