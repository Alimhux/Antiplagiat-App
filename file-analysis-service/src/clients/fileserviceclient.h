#ifndef FILESERVICECLIENT_H
#define FILESERVICECLIENT_H

#pragma once

#include <string>
#include <vector>

namespace clients {

// Информация о файле из file-storing-service
struct FileInfo {
  int id;
  std::string studentName;
  std::string taskId;
  std::string filename;
  std::string uploadedAt;
};

class FileServiceClient {
public:
  explicit FileServiceClient(const std::string& baseUrl);

  // Найти файлы по хэшу
  std::vector<FileInfo> findByHash(const std::string& hash);

private:
  std::pair<std::string, int> parseUrl(const std::string& url);

  std::string baseUrl_;
  std::string host_;
  int port_;
};

}


#endif //FILESERVICECLIENT_H
