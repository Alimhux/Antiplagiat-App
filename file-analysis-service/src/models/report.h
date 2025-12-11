#ifndef REPORT_H
#define REPORT_H

#include <string>
#include <optional>

namespace models {

struct Report {
  int id = 0;
  int submissionId = 0;
  std::string taskId;
  std::string studentName;
  bool isPlagiarism = false;
  double similarityPercent = 0.0;
  std::optional<int> originalSubmissionId;
  std::string status;
  std::string createdAt;
  std::string completedAt;
};

}

#endif //REPORT_H
