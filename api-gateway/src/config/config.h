#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace config {

struct ServerConfig {
  int port;
  std::string fileServiceUrl;
  std::string analysisServiceUrl;
};

class Config {
public:
  static Config& instance();

  const ServerConfig& server() const;

private:
  Config();
  static std::string getEnv(const char* name, const char* defaultValue);

  ServerConfig server_;
};

}
#endif //CONFIG_H
