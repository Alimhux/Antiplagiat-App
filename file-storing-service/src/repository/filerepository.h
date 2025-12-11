#ifndef FILEREPOSITORY_H
#define FILEREPOSITORY_H

#include "../db/database.h"
#include "../models/submission.h"
#include <vector>
#include <optional>

namespace repository {

class FileRepository {
public:
  explicit FileRepository(db::Database& database);

  // Создать новую запись о загруженном файле
  int create(const models::Submission& submission);

  // Найти по ID
  std::optional<models::Submission> findById(int id);

  // Найти все файлы с указанным хэшем
  std::vector<models::Submission> findByHash(const std::string& hash);

  // Найти все файлы для задания
  std::vector<models::Submission> findByTaskId(const std::string& taskId);

private:
  models::Submission rowToSubmission(const pqxx::row& row);

  db::Database& db_;
};

}


#endif //FILEREPOSITORY_H
