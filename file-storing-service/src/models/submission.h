#ifndef SUBMISSION_H
#define SUBMISSION_H

#include <string>
#include <cstdint>

namespace models {

struct Submission {
  int id = 0;
  std::string studentName;
  std::string taskId;
  std::string filename;
  std::string filePath;
  std::string fileHash;
  int64_t fileSize = 0;
  std::string uploadedAt;
};
}

#endif //SUBMISSION_H
