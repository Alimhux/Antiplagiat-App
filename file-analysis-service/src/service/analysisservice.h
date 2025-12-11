#ifndef ANALYSISSERVICE_H
#define ANALYSISSERVICE_H



#include "../repository/reportrepository.h"
#include "../clients/fileserviceclient.h"
#include "../models/report.h"
#include <string>
#include <vector>
#include <optional>

namespace service {

// Запрос на анализ
struct AnalyzeRequest {
  int submissionId;
  std::string taskId;
  std::string studentName;
  std::string fileHash;
};

// Результат анализа
struct AnalyzeResult {
  int reportId;
  int submissionId;
  bool isPlagiarism;
  double similarityPercent;
  std::optional<int> originalSubmissionId;
  std::string status;
};

class AnalysisService {
public:
  AnalysisService(repository::ReportRepository& repo, clients::FileServiceClient& fileClient);

  AnalyzeResult analyze(const AnalyzeRequest& request);

  std::optional<models::Report> getReport(int submissionId);

  std::vector<models::Report> getReportsByTask(const std::string& taskId);

private:
  repository::ReportRepository& repo_;
  clients::FileServiceClient& fileClient_;
};

}


#endif //ANALYSISSERVICE_H
