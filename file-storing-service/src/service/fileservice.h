#ifndef FILESERVICE_H
#define FILESERVICE_H

#include "../repository/filerepository.h"
#include "../models/submission.h"
#include <string>
#include <vector>
#include <optional>

namespace service {

// Запрос на загрузку файла
struct UploadRequest {
  std::string studentName;
  std::string taskId;
  std::string filename;
  std::string content;
};

// Результат загрузки
struct UploadResult {
  int id;
  std::string studentName;
  std::string taskId;
  std::string filename;
  std::string fileHash;
  int64_t fileSize;
};

class FileService {
public:
  FileService(repository::FileRepository& repo, const std::string& uploadPath);

  // Загрузить новый файл
  UploadResult uploadFile(const UploadRequest& request);

  // Получить информацию о файле
  std::optional<models::Submission> getSubmission(int id);

  // Получить содержимое файла
  std::string getFileContent(int id);

  // Найти файлы по хэшу
  std::vector<models::Submission> findByHash(const std::string& hash);

  // Найти файлы по заданию
  std::vector<models::Submission> findByTaskId(const std::string& taskId);

private:
  void saveToFile(const std::string& path, const std::string& content);
  std::string readFromFile(const std::string& path);

  repository::FileRepository& repo_;
  std::string uploadPath_;
};

}



#endif //FILESERVICE_H
