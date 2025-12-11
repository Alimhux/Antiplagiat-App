#ifndef DATABASE_H
#define DATABASE_H


#include <pqxx/pqxx>
#include <memory>
#include <string>

namespace db {

class Database {
public:
  explicit Database(const std::string& connectionString,
                    int maxRetries = 10,
                    int retryDelaySeconds = 2);
  ~Database();

  // Non-copyable
  Database(const Database&) = delete;
  Database& operator=(const Database&) = delete;

  // Movable
  Database(Database&&) noexcept;
  Database& operator=(Database&&) noexcept;

  pqxx::connection& connection();
  bool isConnected() const;

private:
  void connect();
  void reconnect();

  std::string connectionString_;
  int maxRetries_;
  int retryDelaySeconds_;
  std::unique_ptr<pqxx::connection> conn_;
};

}

#endif //DATABASE_H
