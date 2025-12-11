#ifndef REPORTREPOSITORY_H
#define REPORTREPOSITORY_H

#include "../db/database.h"
#include "../models/report.h"
#include <vector>
#include <optional>
#include <pqxx/pqxx>

namespace repository {

class ReportRepository {
public:
  explicit ReportRepository(db::Database& database);

  // Создать новый отчёт
  int create(const models::Report& report);

  // Найти по ID submission
  std::optional<models::Report> findBySubmissionId(int submissionId);

  // Найти все отчёты по заданию
  std::vector<models::Report> findByTaskId(const std::string& taskId);

private:
  models::Report rowToReport(const pqxx::row& row);

  db::Database& db_;
};

}

#endif //REPORTREPOSITORY_H
