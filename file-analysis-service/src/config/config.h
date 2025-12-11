#ifndef CONFIG_H
#define CONFIG_H


#include <string>

namespace config {

struct DatabaseConfig {
  std::string host;
  std::string port;
  std::string name;
  std::string user;
  std::string password;

  std::string connectionString() const;
};

struct ServerConfig {
  int port;
  std::string fileServiceUrl;
};

class Config {
public:
  static Config& instance();

  const DatabaseConfig& database() const;
  const ServerConfig& server() const;

private:
  Config();
  static std::string getEnv(const char* name, const char* defaultValue);

  DatabaseConfig db_;
  ServerConfig server_;
};

}



#endif //CONFIG_H
